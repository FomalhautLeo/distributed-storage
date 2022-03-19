#include <string>
#include <thread>
#include <unistd.h>

#include "CacheServer/CacheChange.h"
#include "CacheServer/CacheLocalMasterBackup.h"

bool CacheChange::busy_ = false;
std::string CacheChange::status_ = {'n','n'};
std::vector<std::unique_ptr<Listener> > CacheChange::listen_array_{};
std::string CacheChange::ip_to_send_ = "";
std::string CacheChange::ips_to_send_ = "";
std::vector<std::string> CacheChange::keys_to_delete_{};


unsigned short CacheChange::this_index_ = 0;


/**
 * @brief Prepare to Expand or Contract, Set some listener and change state machine
 * 
 * @param code 'e' for Cache Expand, 'c' for CacheContract
 * @param message_from_master received message from master
 * @param cache_num CacheNum of this cacheServer
 * @param is_prim whether the cacheServer is primary cache
 */
void CacheChange::PrepareChange(char code,
                                const std::string& message_from_master,
                                int cache_num,
                                bool is_prim) {
  CacheLocalMasterBackup::UpdateHashSlot(mt::Get1stStr(message_from_master));
  int change_info = std::stoi(mt::Get2ndStr(message_from_master).substr(0, 2));
  if (code == 'e') {
    if (change_info % 10 == cache_num) {
      SetListenS(change_info / 10, is_prim);
    } else {
      ip_to_send_ = mt::Get2ndStr(message_from_master).substr(2);
      status_ = {'e','n'};
    }
  } else {
    if (change_info % 10 != cache_num) {
      busy_ = true;
      status_ = {'c','l'};
      SetListenS(1, is_prim);
    } else {
      ips_to_send_ = mt::Get2ndStr(message_from_master).substr(2);
      status_ = {'c','n'};
    }
  }
}

/**
 * @brief Auxilary function
 *
 */

void CacheChange::InfoSplit(const std::string& str,
                            unsigned short& res_first,
                            std::string& res_second,
                            unsigned short& res_third,
                            const char split_first = '+',
                            const char split_second = '-',
                            const char split_third = ':') {
	if (str.empty())
    return;
	std::string strs = str + split_first;
	auto first_split = strs.find(split_first);

	std::string temp = strs.substr(0, first_split);
  auto second_split = temp.find(split_second);
	auto third_split = temp.find(split_third);

	res_first = std::stoi(temp.substr(0,second_split));
  res_second = temp.substr(second_split + 1, third_split - 2);
	res_third = std::stoi(temp.substr(third_split+1));

	strs = strs.substr(first_split + 1, strs.size());
	first_split = strs.find(split_first);
}

/**
 * @brief Set Some listeners for incoming transition
 * 
 * @param cache_number how many listeners should be set
 * @param is_prim whether this cache is primary cache
 */

void CacheChange::SetListenS(
    const unsigned short& cache_number, const bool& is_prim) {
  if (listen_array_.size() > 0)
    listen_array_.clear();
  if (status_[0] == 'e') {
    // port is different for different cache
    unsigned short prim_port = change_listen_port_ + (is_prim ? 0 : 100); 
    for (auto index = 0; index < cache_number; ++index) {
      listen_array_.emplace_back(std::make_unique<Listener>());
      listen_array_[index]->SetListen(prim_port + index);
    }
  } else {
    // port is different for different cache   
    unsigned short listen_port = change_listen_port_ + (is_prim ? 0 : 100) + this_index_;
    listen_array_.emplace_back(std::make_unique<Listener>());
    listen_array_[0]->SetListen(listen_port);
  }
}

/**
 * @brief Universal function for Cache Expand and Contract both
 *        In Cache Expand, used for caches to send data to new cache
 *        In Cache Contract, used for deleted cache to send data to others
 * 
 * @param lru_cache self LRU
 * @param keys_to_delete vectors to store sent data
 * @param cache_info the destination info
 * @param is_prim whether the cache is primary
 */

void CacheChange::SendToCache(const std::unique_ptr<LRUCache>& lru_cache,
                              std::vector<std::string>& keys_to_delete,
                              const std::string& cache_info,
                              bool is_prim) {
  unsigned short cache_index;
  std::string cache_ip;
  unsigned short cache_port;
  //Extract information from message from master
  InfoSplit(cache_info,cache_index,cache_ip,cache_port);
  cache_port = cache_port + (is_prim ? 0 : 100);
  std::unique_ptr<ActiveSocket> socket_to_cache = std::make_unique<ActiveSocket>();
  socket_to_cache->ConnectToServer(cache_ip,cache_port);
  std::string to_be_sent = "";

  // Make sure no request from client is ongoing
  while (lru_cache->GetBusy()) {  std::this_thread::yield(); }
  // Traverse the LRU
  const auto& lru_queue = lru_cache->GetList();
  auto lru_queue_rbegin = lru_queue.crbegin();
  auto lru_queue_rend = lru_queue.crend();

  unsigned short slot_location = 0;
  for (auto reverse_it = lru_queue_rbegin; reverse_it != lru_queue_rend; ++reverse_it) {
    // Get new location for k-v
    slot_location = CacheLocalMasterBackup::GetCacheIndex(reverse_it->first,size_of_k_);
    if (slot_location == cache_index) {
      to_be_sent += reverse_it->first + reverse_it->second;
      keys_to_delete.emplace_back(reverse_it->first);
    }
  }

  socket_to_cache->SendMsg(to_be_sent);
  // End message to notify the receiver
  socket_to_cache->SendMsg("1");
  socket_to_cache->Close();
}

/**
 * @brief Universal function for Cache Expand and Contract both
 * 
 * @param server_to_cache Listener for incoming transition
 * @param lru_cache Depends on caller, in Expand, it is self LRU, in Contract, it is a temp LRU
 * @param prom Promise for return result in multi-thread
 * 
 */

std::string CacheChange::ReceiveFromCache(
    const std::unique_ptr<Listener>& server_to_cache,
    const std::unique_ptr<LRUCache>& lru_cache,
    std::promise<char>& prom) {
  auto socket_to_cache = server_to_cache->AcceptConn(nullptr);
  std::string received_message = socket_to_cache->RecvMsg();
  auto buf_size = received_message.size();
  // Fail if received message is empty
  if (buf_size == 0) {
      socket_to_cache->Close();
      server_to_cache->Close();
      prom.set_value('0');
      return "0";
  }
  // Split and store the message from cache
  auto part_number = 0;
  auto start_point = 0;
  part_number = static_cast<unsigned short>(buf_size / size_of_kv_);
  for (int index = 0; index < part_number; index++) {
    start_point = index * size_of_kv_;
    lru_cache->Store(received_message.substr(start_point,size_of_k_),
                     received_message.substr(start_point + size_of_k_,size_of_v_));
  }
  socket_to_cache->Close();
  server_to_cache->Close();
  prom.set_value('1');
  return "1";
}

/**
 * @brief Universal function in cache Expand and Contract both. Set state machine and delete sent keys
 * 
 * @param lru_cache self LRU
 */

void CacheChange::MigrationConfirmed(const std::unique_ptr<LRUCache>& lru_cache) {
  // Restore the LRU capacity
  lru_cache->UnlockCapacity();
  CacheLocalMasterBackup::ConfirmPossibleHashSlot();
  listen_array_.clear();
  if (keys_to_delete_.empty())
    return;
  while (lru_cache->GetBusy()) {  std::this_thread::yield(); }
  // start to delete sent data
  auto& lru_queue = lru_cache->GetList();
  auto& hash_table = lru_cache->GetTable();
  auto it = hash_table.begin();
  for (const auto& val : keys_to_delete_) {
    it = hash_table.find(val);
    if (it != hash_table.end()) {
      lru_queue.erase(it->second);
      hash_table.erase(val);
    }
  }
  keys_to_delete_.clear();
}
bool CacheChange::GetBusy() { return busy_; }
void CacheChange::SetBusy(bool param) { busy_ = param; }
std::string CacheChange::GetStatus() {return status_; }
void CacheChange::SetStatus(const std::string& param) {status_ = param;};
const std::vector<std::unique_ptr<Listener> >& CacheChange::GetListenArray() {
  return listen_array_;
}
const std::string& CacheChange::GetIpToSend() { return ip_to_send_; }
const std::string& CacheChange::GetIpSToSend() { return ips_to_send_; }
std::vector<std::string>& CacheChange::GetKeysToDelete() { return keys_to_delete_; }


/**
 * @brief Function called in cache Expand. It can start the data migration for different role in Expand
 *        If this cache is new cache, call ReceiveFromOlds, else call SendToCache
 * @param lru_cache self LRU
 * @param socket_to_master socket for send message to Master
 * @param is_prim whether the cache is primary
 */

void CacheChange::CacheExpand::StartExpand(
    const std::unique_ptr<LRUCache>& lru_cache,
    const std::unique_ptr<ActiveSocket>& socket_to_master,
    bool is_prim) {
  lru_cache->LockCapacity();
  if (status_[1] == 'l') {
    std::thread receive_thread(ReceiveFromOldS,
                               std::cref(lru_cache),
                               std::cref(socket_to_master));
    receive_thread.detach();
  } else {
    std::thread send_thread(SendToCache,
                            std::cref(lru_cache),
                            std::ref(GetKeysToDelete()),
                            GetIpToSend(),
                            is_prim);
    send_thread.detach();
  }
}

/**
 * @brief This function is called when new cache start to receive from other caches. It will receive
 *        from several caches simultaneously and ReceiveFromCache is used as an assistance. 
 * 
 * @param lru_cache self LRU or a temp LRU, use self LRU as the first receiver and temp for others
 * @param socket_to_master socket to send message to Master
 */
void CacheChange::CacheExpand::ReceiveFromOldS(
  const std::unique_ptr<LRUCache>& lru_cache,
  const std::unique_ptr<ActiveSocket>& socket_to_master) {
  const auto& listen_array = GetListenArray();
  const auto old_cache_number = listen_array.size();
  std::vector<std::unique_ptr<LRUCache> > LRUCache_vector(old_cache_number - 1);
  std::vector<std::unique_ptr<std::thread> > thread_vector(old_cache_number);
  std::vector<std::promise<char> > promise_vector(old_cache_number);
  std::vector<std::future<char> > future_vector(old_cache_number);
  for (auto index = 0; index < old_cache_number; ++index)
    future_vector[index] = promise_vector[index].get_future();
  for (auto index = 0; index < old_cache_number; ++index) {
    // For the first thread, use self LRU to receive and store data
    if (index == 0) {
    thread_vector[index] = std::make_unique<std::thread>(
        ReceiveFromCache,
        std::cref(listen_array[index]),
        std::cref(lru_cache),
        std::ref(promise_vector[index]));
        continue;      
    }
    // For other threads, use a temp LRU
    LRUCache_vector[index - 1] = std::make_unique<LRUCache>(-1);
    thread_vector[index] = std::make_unique<std::thread>(
        ReceiveFromCache,
        std::cref(listen_array[index]),
        std::cref(LRUCache_vector[index - 1]),
        std::ref(promise_vector[index]));
  }
  std::string receive_status = "";
  for (auto index = 0; index < old_cache_number; ++index) {
    thread_vector[index]->join();
    receive_status += future_vector[index].get();
  }
  while (lru_cache->GetBusy()) {   std::this_thread::yield(); }
  // merge all the LRU
  auto& lru_queue = lru_cache->GetList();
  auto& hash_table = lru_cache->GetTable();
  for (auto index = 0; index < old_cache_number - 1; index++) {
    if (receive_status[index] == '1') {
      hash_table.insert(LRUCache_vector[index]->GetTable().begin(),
                        LRUCache_vector[index]->GetTable().end());
      lru_queue.splice(lru_queue.end(),LRUCache_vector[index]->GetList());
    }
  }
  socket_to_master->SendMsg(MGR_COMP,receive_status);
}


std::unordered_map<unsigned short, std::string>
    CacheChange::CacheContract::multiple_data_to_send_{};

/**
 * @brief Function called in cache Contract. It can start the data migration for different role in Contract
 *        If this cache is deleted cache, call SendToAliveCaches, else call ReceiveFromCache
 * @param lru_cache self LRU
 * @param socket_to_master socket for send message to Master
 * @param is_prim whether the cache is primary
 */

void CacheChange::CacheContract::StartContract(
    const std::unique_ptr<LRUCache>& lru_cache,
    const std::unique_ptr<ActiveSocket>& socket_to_master,
    bool is_prim) {
  if (status_[1] == 'l')
    CacheContract::ReceiveFromDying(lru_cache,socket_to_master);
  else
    CacheContract::SendToAliveCacheS(lru_cache,is_prim);
}

/**
 * @brief Auxilary function
 * 
 */

void CacheChange::CacheContract::StringSplit(const std::string& str,
                                             std::vector<std::string>& res_first,
                                             const char split_first = '+') {
	if (str.empty())
    return;
	std::string strs = str + split_first;
	auto first_split = strs.find(split_first);
	while (first_split != strs.npos) {
    res_first.emplace_back(strs.substr(0, first_split));
    strs = strs.substr(first_split + 1, strs.size());
    first_split = strs.find(split_first);
	}
}


/**
 * @brief This function is called when deleted cache send all of its data to others
 *        It will first traverse all the K-V and category the K-V for different new owners
 *        and it will send to them seperatively.
 * 
 * @param lru_cache self LRU
 * @param is_prim whether the cache is primary
 */
void CacheChange::CacheContract::SendToAliveCacheS(
    const std::unique_ptr<LRUCache>& lru_cache, bool is_prim) {
  std::vector<std::string> infos;
  auto ips_to_send = GetIpSToSend();
  // Split multiple information from Master
  StringSplit(ips_to_send,infos);
  

  auto still_alive_cache_number = infos.size();
  
  // Set the LRU to be busy, refuse all the request from client
  busy_ = true;

  auto lru_queue_rbegin = lru_cache->GetList().crbegin();
  auto lru_queue_rend = lru_cache->GetList().crend();

  // Traverse all of the data, category the new location of each k-v
  unsigned short slot_location = 0;
  for (auto reverse_it = lru_queue_rbegin; reverse_it != lru_queue_rend; ++reverse_it) {
    slot_location = CacheLocalMasterBackup::GetCacheIndex(reverse_it->first,size_of_k_);
    multiple_data_to_send_[slot_location] += (reverse_it->first + reverse_it->second);
  }
  
  unsigned short cache_index;
  std::string cache_ip;
  unsigned short cache_port;
  unsigned short add_prim = is_prim ? 0 : 100;
  for (auto index = 0; index < still_alive_cache_number; ++index) {
  InfoSplit(infos[index],cache_index,cache_ip,cache_port);
  cache_port = cache_port + add_prim;
  std::unique_ptr<ActiveSocket> socket_to_cache = std::make_unique<ActiveSocket>();
  socket_to_cache->ConnectToServer(cache_ip,cache_port);
  // Send data to correspond cache
  socket_to_cache->SendMsg(multiple_data_to_send_.at(cache_index));
  socket_to_cache->SendMsg("1");
  socket_to_cache->Close();
  }
}

/**
 * @brief This function is for non-deleted cache in Contract to receive data from deleted cache
 * 
 * @param lru_cache this LRU
 * @param to_master socket for send message to Master
 */

void CacheChange::CacheContract::ReceiveFromDying(const std::unique_ptr<LRUCache>& lru_cache, const std::unique_ptr<ActiveSocket>& to_master) {
  std::promise<char> prom;
  if (ReceiveFromCache(GetListenArray()[0], lru_cache, prom) == "1")
    to_master->SendMsg(MGR_COMP);
  else
    to_master->SendMsg(MGR_FAIL);
  busy_ = false;
  status_ = {'n','n'};
  return;
}
