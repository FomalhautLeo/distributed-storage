#include <fstream>
#include <stdlib.h>
#include <sys/file.h>
#include <thread>
#include <unistd.h>

#include "Client/ClientListener.h"
#include "Client/ClientLocalMasterBackup.h"
#include "Client/ClientManager.h"
#include "Client/TestDataGeneration.h"
#include "Config/Logger.h"
#include "Config/MessageType.h"
#include "Config/NetworkInfo.h"

ClientManager::ClientManager(int speed) {
  ni_ = NetworkInfo::GetInstance();
  times_ = 1e6 / speed;
  hash_slot_.clear();
  to_master_ = std::make_unique<ClientSocket>();
  to_caches_["11"] = std::make_unique<ClientSocket>();
  to_caches_["21"] = std::make_unique<ClientSocket>();
  to_caches_["12"] = std::make_unique<ClientSocket>();
  to_caches_["22"] = std::make_unique<ClientSocket>();
  to_caches_["13"] = std::make_unique<ClientSocket>();
  to_caches_["23"] = std::make_unique<ClientSocket>();
  // Default "11", "12" and "13" are primary nodes.
  eff_caches_ = std::unordered_map<char, std::string> {
    {'1', "11"},
    {'2', "12"},
    {'3', "13"}
  };
  ServerLog::SetLogPath("Log.log");
}

void ClientManager::Run() {
  // Connect to all of servers.
  this->Initialize();
  this->ComWithMaster();
}

/**
 * @brief Initialize the Client, Connect several nodes in the system
 *
 */

void ClientManager::Initialize() {
  LogInfo << "=== Welcome to use distributed storage system ===" << std::endl;
  LogInfo << "=== Initializing..." << std::endl;
  TestDataGeneration::Initialize();
  ClientLocalMasterBackup::GenerateTableForCrc16();
  // Connect to master and caches.
  while (!to_master_->ConnectToServer(ni_->MASTER_IP, ni_->MASTER_PORT, false))
    sleep(1);
  // Tell master this is a client node.
  to_master_->SendMsg(IAM_CLNT);
  // Get the hash slot
  std::string received_hash = mt::Get1stStr(to_master_->RecvMsg());
  ClientLocalMasterBackup::UpdateHashSlot(received_hash);
  LogInfo << "Connect to Master successfully!" << std::endl;
  for (auto& it : to_caches_) {
    while (!it.second->ConnectToServer(
        ni_->CACHE_IP.at(it.first), ni_->CACHE_PORT.at(it.first), false)) {
      sleep(1);
    }
    it.second->SendMsg(IAM_CLNT);
  }
  LogInfo << "Connect to CacheServer successfully!" << std::endl;
  this->RequestHandler();
}

/**
 * @brief allow USER to choose which role the client is.
 *
 */

void ClientManager::RequestHandler() {
  while (true) {
    this->ShowMenu();
    std::cin.clear();
    std::cin.sync();
    std::cin >> select_;
    if (select_ == '1' || select_ == '2')
      break;
    LogWarn << "Wrong input, please try again." << std::endl;
  }
  switch (select_) {
    // Write client
    case '1': {
      // Commute with read client
      std::thread commute_with_read = std::thread(&ClientManager::ComWithRead,this);
      commute_with_read.detach();
      // Start Continuous store data to caches
      std::thread get_thread = std::thread(&ClientManager::StoreData,this);
      get_thread.detach();
      break;
    }
    // Read Client
    case '2': {
      // Commute with write client
      std::thread commute_with_write = std::thread(&ClientManager::ComWithWrite,this);
      commute_with_write.detach();
      // Start Continuous read data to caches
      std::thread put_thread = std::thread(&ClientManager::GetData,this);
      put_thread.detach();
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Receive orders and send feedback with Master
 *
 */

void ClientManager::ComWithMaster() {
  std::string msg_from_master;
  std::string primary_index;
  std::string standby_index;
  std::string to_be_delete = "";
  while(true) {
    msg_from_master = to_master_->RecvMsg();
    switch(mt::GetMsgType(msg_from_master)) {
      // The system is about to Expand
      case EXP_LSTN: {
        exp_or_con_ = 'e';
        // update hash slot
        ClientLocalMasterBackup::UpdateHashSlot(mt::Get1stStr(msg_from_master));
        use_new_ = true;
        cache_index_string_ = mt::Get2ndStr(msg_from_master);
        primary_index = "1" + cache_index_string_;
        standby_index = "2" + cache_index_string_;
        // update map for caches
        to_caches_[primary_index] = std::make_unique<ClientSocket>();
        to_caches_[standby_index] = std::make_unique<ClientSocket>();
        eff_caches_[cache_index_string_[0]] = primary_index;
        // trying to connect the new cache, primary and standby both
        while (!to_caches_[primary_index]->ConnectToServer(
        ni_->CACHE_IP.at(primary_index), ni_->CACHE_PORT.at(primary_index), false)) {
          sleep(1);
        }
        to_caches_[primary_index]->SendMsg(IAM_CLNT);
        while (!to_caches_[standby_index]->ConnectToServer(
        ni_->CACHE_IP.at(standby_index), ni_->CACHE_PORT.at(standby_index), false)) {
          sleep(1);
        }
        to_caches_[standby_index]->SendMsg(IAM_CLNT);
        to_master_->SendMsg(REC_HASH);
        break;
      }
      // The system is about to Contract
      case CON_LSTN: {
        to_master_->SendMsg(REC_HASH);
        exp_or_con_ = 'c';
        // block the request when update the hash slot
        hash_set_busy_ = true;
        ClientLocalMasterBackup::UpdateHashSlot(mt::Get1stStr(msg_from_master));
        use_new_ = true;
        to_be_delete = mt::Get2ndStr(msg_from_master);
        hash_set_busy_ = false;
        break;
      }
      // Data migration finished
      case MGR_COMP: {
        hash_set_busy_ = true;
        ClientLocalMasterBackup::ConfirmPossibleHashSlot();
        use_new_ = false;
        hash_set_busy_ = false;
        if (!to_be_delete.empty()) {
          to_caches_["1" + to_be_delete]->Close();
          to_caches_.erase("1" + to_be_delete);
          to_caches_["2" + to_be_delete]->Close();
          to_caches_.erase("2" + to_be_delete);
          to_be_delete.clear();
        } else {
          primary_index.clear();
          standby_index.clear();
        }
        if (select_ == '1' && !keys_to_do_.empty()) {
          usleep(50000);
          to_do_busy_ = true;
        } else if (select_ == '1' && keys_to_do_.empty()) {
          if (exp_or_con_ == 'e'){
          // Notify the cache to set the lru unbusy
          to_caches_[primary_index]->SendMsg(SET_BUSY);
          to_caches_[standby_index]->SendMsg(SET_BUSY);
          }
        }
        break;
      }
    }
  }
}

void ClientManager::ShowMenu() {
  LogWarn << "1. Store Data" << std::endl;
  LogWarn << "2. Get Data"   << std::endl;
  LogWarn << "Please enter your selection: ";
}



/**
 * @brief Continuous Send Data to correspond CacheServer
 *
 */

void ClientManager::StoreData() {
  std::ofstream outFile("keylist.txt",std::ios::out);
  if (!outFile.is_open()) {
    LogError << "Error! Can not create keylist.txt"<<std::endl;
    exit(1);
  }

  /**
   * @brief Get the fd of ofstream
   * 
   */
  auto helper = [](std::filebuf& fb) -> int {
    class Helper : public std::filebuf {
    public:
      int handle() { return _M_file.fd(); }
    };
    return static_cast<Helper&>(fb).handle();
  };

  int write_fd = helper(*outFile.rdbuf());
  std::string key(size_of_k_,' ');
  std::string value(size_of_v_,' ');
  std::string cache_num;

  auto fake_accumulator = 0;
  bool second_write = false;
  while (true) {
    // Add fake key periodic
    if (++fake_accumulator == fake_round_) {
      TestDataGeneration::GetNum(key);
      // Use flock to support multiple clients
      flock(write_fd,LOCK_EX);
      outFile<<std::endl;
      outFile<<key;
      outFile.flush();
      flock(write_fd,LOCK_UN);
      LogWarn << "Fake key has been written." <<std::endl;
      fake_accumulator = 0;
    }
    // Failed to send message. Switch to standby node.
    std::string msg_from_cache;
    // If the migration is finished, deal with delayed requests
    if (to_do_busy_) {
      if (exp_or_con_ == 'e') {
        to_caches_["1" + cache_index_string_]->SendMsg(SET_BUSY);
        to_caches_["2" + cache_index_string_]->SendMsg(SET_BUSY);
        sleep(1);
      }
      LogWarn << "Start Dealing with delayed request" <<std::endl;
      int keys_to_do_size = keys_to_do_.size();
      for (auto index = 0; index < keys_to_do_size; ++index) {
        key = keys_to_do_[index];
        value = values_to_do_[index];
        cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
            key,size_of_k_, use_new_)) + '0'];
        LogInfo << "To: " << cache_num << std::endl;
        if (to_caches_[cache_num]->SendMsg(PUT_DATA, key, value) < 0
            || (msg_from_cache = to_caches_[cache_num]->RecvMsg()).empty()) {
          cache_num = this->SwitchCache(cache_num);
          LogInfo << "To: " << cache_num << std::endl;
          to_caches_[cache_num]->SendMsg(PUT_DATA, key, value);
          msg_from_cache = to_caches_[cache_num]->RecvMsg();
        }
        // Get reply from cache.
        if (!msg_from_cache.empty()) {
          if (mt::GetMsgType(msg_from_cache) == STO_SUCC) {
            LogInfo << "Store data successfully." << std::endl;
            // Use flock to support multiple clients
            flock(write_fd,LOCK_EX);
            outFile<<std::endl;
            outFile<<key;
            outFile.flush();
            flock(write_fd,LOCK_UN);
            second_write = true;
            usleep(times_ / 10);
          }
        } else {
          LogError << "Store failed." << std::endl;
        }
      }
      keys_to_do_.clear();
      keys_to_do_.shrink_to_fit();
      values_to_do_.clear();
      values_to_do_.shrink_to_fit();
      to_do_busy_ = false;
      LogWarn << "Delayed Request Finished!" <<std::endl;
      sleep(1);
      to_do_finish_ = true;
    }
    TestDataGeneration::GetNum(key);
    TestDataGeneration::GetNum(value);
    // block the request when updating the hashslot
    while (hash_set_busy_)
      std::this_thread::yield();
    cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
        key,size_of_k_, use_new_)) + '0'];
    if (to_caches_[cache_num] == nullptr) {
      while (hash_set_busy_) { std::this_thread::yield();}
      cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
          key,size_of_k_, use_new_)) + '0'];
    }
    LogInfo << "To: " << cache_num << std::endl;
    if (to_caches_[cache_num]->SendMsg(PUT_DATA, key, value) < 0
        || (msg_from_cache = to_caches_[cache_num]->RecvMsg()).empty()) {
      cache_num = this->SwitchCache(cache_num);
      LogInfo << "To: " << cache_num << std::endl;
      to_caches_[cache_num]->SendMsg(PUT_DATA, key, value);
      msg_from_cache = to_caches_[cache_num]->RecvMsg();
    }

    // Get reply from cache.
    if (!msg_from_cache.empty()) {
      if (mt::GetMsgType(msg_from_cache) == STO_SUCC) {
        LogInfo << "Store data successfully." << std::endl;
        // Use flock to support multiple clients
        flock(write_fd,LOCK_EX);
        if (second_write)
          outFile<<std::endl;
        outFile<<key;
        outFile.flush();
        flock(write_fd,LOCK_UN);
        second_write = true;
        usleep(times_);
        // LRU is busy, if is during the Expand get the request to a temp vector, deal with it later
      } else if (mt::GetMsgType(msg_from_cache) == LRU_BUSY) {
        LogWarn << "LRU is writing, this request will be delayed" <<std::endl;
        usleep(times_ << 1);
        if (exp_or_con_ == 'e') {
          keys_to_do_.emplace_back(key);
          values_to_do_.emplace_back(value);
        } else {
          // LRU is busy, if is during the Contract, just block the request
          while(mt::GetMsgType(msg_from_cache) == LRU_BUSY) {
            if (to_caches_[cache_num]->SendMsg(PUT_DATA, key, value) < 0
                || (msg_from_cache = to_caches_[cache_num]->RecvMsg()).empty()) {
              cache_num = this->SwitchCache(cache_num);
              LogInfo << "To: " << cache_num << std::endl;
              to_caches_[cache_num]->SendMsg(PUT_DATA, key, value);
              msg_from_cache = to_caches_[cache_num]->RecvMsg();
            }
            usleep(100000);
          }
        }
      }
    }
  }
}

/**
 * @brief Get Key from keylist.txt, Send read request to correspond CacheServer and show the feedback
 *
 */

void ClientManager::GetData() {
  std::ifstream inFile("keylist.txt",std::ios::in);
  while (!inFile.is_open())
    sleep(1);
  auto helper = [](std::filebuf& fb) -> int {
    class Helper : public std::filebuf {
    public:
      int handle() { return _M_file.fd(); }
    };
    return static_cast<Helper&>(fb).handle();
  };
  int read_fd = helper(*inFile.rdbuf());
  std::string key;
  std::string nullstring;
  std::string cache_num;
  while (true) {
    std::string msg_from_cache;
    if (to_do_busy_) {
      LogWarn << "Start Dealing with delayed request" <<std::endl;
      for (const auto& key : keys_to_do_) {
        cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
            key,size_of_k_, use_new_)) + '0'];
        if (to_caches_[cache_num]->SendMsg(GET_DATA, key) < 0
            || (msg_from_cache = to_caches_[cache_num]->RecvMsg()).empty()) {
          cache_num = this->SwitchCache(cache_num);
          to_caches_[cache_num]->SendMsg(GET_DATA, key);
          msg_from_cache = to_caches_[cache_num]->RecvMsg();
        }
        // Get reply from cache.
        if (!msg_from_cache.empty()) {
          if (mt::GetMsgType(msg_from_cache) == RET_VALE) {
            LogWarn << "From: "  << cache_num << std::endl;
            LogInfo << "Key: " << key <<std::endl;
            LogInfo << "Value: " << mt::Get1stStr(msg_from_cache) << std::endl;
            usleep(times_ / 10);
          } else {
            LogError << "Read failed." <<std::endl;
          }
        }
      }
      keys_to_do_.clear();
      keys_to_do_.shrink_to_fit();
      to_do_busy_ = false;
      LogWarn << "Delayed Request finished!" <<std::endl;
    }
    while (inFile.eof()) {
      sleep(1);
      LogInfo << "Reaching the end of keylist, waiting for update..." << std::endl;
      inFile.seekg(-1,std::ios::cur);
      // Use flock to support multiple clients
      flock(read_fd,LOCK_SH);
      inFile.sync();
      flock(read_fd,LOCK_UN);
      inFile>>nullstring;
    }
    inFile >> key;

    while (hash_set_busy_) {
      std::this_thread::yield();
    }
    cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
        key,size_of_k_, use_new_)) + '0'];

    if (to_caches_[cache_num] == nullptr) {
      while (hash_set_busy_) {
        std::this_thread::yield();
    }
    cache_num = eff_caches_[(ClientLocalMasterBackup::GetCacheIndex(
        key,size_of_k_, use_new_)) + '0'];
    }

    if (to_caches_[cache_num]->SendMsg(GET_DATA, key) < 0
        || (msg_from_cache = to_caches_[cache_num]->RecvMsg()).empty()) {
      cache_num = this->SwitchCache(cache_num);
      to_caches_[cache_num]->SendMsg(GET_DATA, key);
      msg_from_cache = to_caches_[cache_num]->RecvMsg();
    }
    // Get reply from cache.
    if (!msg_from_cache.empty()) {
      if (mt::GetMsgType(msg_from_cache) == RET_VALE) {
        LogInfo << "From: "  << cache_num << std::endl;
        LogInfo << "Key: " << key <<std::endl;
        LogInfo << "Value: " << mt::Get1stStr(msg_from_cache) << std::endl;
        usleep(times_);
      } else if (mt::GetMsgType(msg_from_cache) == LRU_BUSY) {
        LogWarn<< "LRU is writing, this request will be delayed" <<std::endl;
        keys_to_do_.emplace_back(key);
        usleep(times_ << 1);
      }
    }
  }
}

/**
 * @brief If the primary CacheServer is down, switch the request to standby CacheServer
 *
 * @param cache_num which CacheServer is down
 * @return std::string return the information of standby CacheServer
 */

std::string ClientManager::SwitchCache(const std::string& cache_num) {
  // Change cache_num.
  std::string new_cache = cache_num;
  new_cache[0] = (cache_num[0] == '1' ? '2' : '1');
  eff_caches_[cache_num[1]] = new_cache;
  // In case the standby node has creashed before.
  to_caches_.at(new_cache)->ConnectToServer(
      ni_->CACHE_IP.at(new_cache), ni_->CACHE_PORT.at(new_cache), true);
  return new_cache;
}

/**
 * @brief If the client is write client, commute with read client to inform the read client
 * deal with delayed request
 *
 */

void ClientManager::ComWithRead() {
  ClientListener listener;
  if (!listener.SetListen(ni_->CLIENT_PORT)) {
    LogError << "Can not bind to CLIENT_PORT!" <<std::endl;
    exit(1);
  }
  auto socket_to_read = listener.AcceptConn(nullptr);
  while (true) {
    while (!to_do_finish_)
      std::this_thread::yield();
    // send the write finish signal to read client, so read client can deal with delayed request
    if (socket_to_read->SendMsg(WRI_FINI) <= 0 ) {
      LogError << "The read client is offline!" << std::endl;
      socket_to_read->Close();
      socket_to_read.reset();
      socket_to_read = listener.AcceptConn(nullptr);
    }
    to_do_finish_ = false;
  }
}

/**
 * @brief If the client is read client, commute with write client to get orders from write client
 *
 */

void ClientManager::ComWithWrite() {
  auto socket_to_write = std::make_unique<ClientSocket>();
  while (!socket_to_write->ConnectToServer(ni_->CLIENT_IP,ni_->CLIENT_PORT,false))
    usleep(500000);
  LogInfo << "Connect to write successfully." << std::endl;
  std::string received_message;
  while (true) {
    // block in recv
    received_message = socket_to_write->RecvMsg();
    if (received_message.empty()) {
      LogError << "The write client is offline!" << std::endl;
      socket_to_write->Close();
      socket_to_write.reset();
      socket_to_write->ConnectToServer(ni_->CLIENT_IP,ni_->CLIENT_PORT,false);
      // Receive th write finish signal, start dealing with delayed requests.
    } else if (mt::GetMsgType(received_message) == WRI_FINI) {
      to_do_busy_ = true;
    }
  }
}
