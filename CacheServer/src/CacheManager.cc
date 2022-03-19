#include <iostream>
#include <queue>
#include <thread>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "CacheServer/CacheChange.h"
#include "CacheServer/CacheLocalMasterBackup.h"
#include "CacheServer/CacheManager.h"
#include "Config/Logger.h"
#include "Config/MessageType.h"

CacheManager::CacheManager(const std::string& my_num_, int capacity)
    : my_num_(my_num_),
      is_primary_(false) {

  cache_ = std::make_unique<LRUCache>(capacity);
  to_master_ = std::make_unique<ActiveSocket>();
  to_primary_ = std::make_unique<ActiveSocket>();
  from_standby_ = nullptr;
  std::string log_path = "Cache" + my_num_ + "Log.log";
  ServerLog::SetLogPath(log_path.c_str());
}

CacheManager::~CacheManager() {
  LogWarn << "Shutting down..." << std::endl;
}

void CacheManager::Run() {
  CacheLocalMasterBackup::GenerateTableForCrc16();
  CacheChange::this_index_ = my_num_[1] - '0';

  this->ConnectToMaster();

  // Standby node need to connect to primary node.
  if (!is_primary_) {
    this->ConnectToPrimary();
    std::thread get_backup(&CacheManager::GetBackupData, this);
    get_backup.detach();
  }
  this->Listen();
}

void CacheManager::StoreData(
    const std::string& key, const std::string& value) const {
  cache_->Store(key, value);
}

std::string CacheManager::GetData(const std::string&& key) const {
  return cache_->Get(key);
}

void CacheManager::ConnectToMaster() {
  // Connect to master first.

  while (!(to_master_->ConnectToServer(ni_->MASTER_IP, ni_->MASTER_PORT))) {
    sleep(1);
  }
  // Skip the message with hashslot.

  // Tell master this is a cache rather than a client.
  to_master_->SendMsg(IAM_CACH, my_num_);
   LogInfo << "Connect to master successfully." << std::endl;
  // Check is this node primary or standby;
  msg_t prim_or_stdb = mt::GetMsgType(to_master_->RecvMsg());
  if (prim_or_stdb == URE_PRIM)
    is_primary_ = true;
  else
    is_primary_ = false;
  std::string msg = to_master_->RecvMsg();
  std::string received_hash = mt::Get1stStr(msg);
  CacheLocalMasterBackup::UpdateHashSlot(received_hash);
  // Create a thread to report heartbeat.
  std::thread report_heartbeat(&CacheManager::ReportHeartbeat, this);
  report_heartbeat.detach();
}

void CacheManager::ConnectToPrimary() {
  // 11 -> 21, 21 -> 11
  std::string primary_num = my_num_;
  primary_num[0] = my_num_[0] == '1' ? '2' : '1';
  while (!to_primary_->ConnectToServer(
      ni_->CACHE_IP.at(primary_num), ni_->CACHE_PORT.at(primary_num)))
    sleep(1);
  to_primary_->SendMsg(IAM_CACH);
  LogInfo << "Connected to primary node successfully." << std::endl;
}

void CacheManager::Listen() {
  // An object for listening
  Listener listener;
  auto listenfd = listener.GetFD();
  int reuse = 1;
  if (setsockopt(listenfd,
                SOL_SOCKET,
                SO_REUSEPORT | SO_REUSEADDR,
                (const void *)&reuse , sizeof(int)) < 0) {
    LogFatal << "Failed to set reuseport." << std::endl;
    //perror(nullptr);
    exit(1);
  }

  listener.SetListen(this->GetPort());
  auto masterfd = to_master_->GetFD();
  const auto MAXEVENTS = 256;
  // Use epoll to handle concurrent request from other nodes
  auto epoll_fd = epoll_create1(0);
  if(epoll_fd == -1){
    LogFatal << "Failed to create epoll_fd." << std::endl;
    //perror(nullptr);
    exit(1);
  }
	epoll_event ev;
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;
  epoll_event events[MAXEVENTS];

  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &ev);
  
  // Specific channel for orders from Master
  ev.data.fd = to_master_->GetFD();
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, masterfd, &ev);

  std::list<std::unique_ptr<ServerSocket> > from_others;

  std::string messages;
  while (true) {
    // Start epoll_wait
    auto nfds = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
    for(int index = 0; index < nfds; ++index) {
      if (events[index].data.fd == listenfd) {
        sockaddr_in addr;
        auto connected = listener.AcceptConn(&addr);
        auto connectfd = connected->GetFD();
        from_others.emplace_back(std::make_unique<ServerSocket>(connected,addr));
        if (connectfd < 0) {
          LogFatal << "Failed to connect to node." << std::endl;
          exit(1);
        }
        ev.data.ptr = (void*)&(from_others.back());
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connectfd, &ev);
      } else if (events[index].data.fd == masterfd) {
        messages = to_master_->RecvMsg();
        const msg_t msg_type=mt::GetMsgType(messages);
        switch (msg_type) {
          // Update to primary node
          case UPG_PMRY:
            is_primary_ = true;
            to_primary_.reset();
            LogInfo << "Update to primary node successfully."<<std::endl;
            break;
          // Prepare to Expand
          case EXP_LSTN:
            CacheChange::PrepareChange('e', messages, my_num_[1] - '0', is_primary_);
            to_master_->SendMsg(REC_HASH);
            break;
          // Prepare to Contract
          case CON_LSTN:
            CacheChange::PrepareChange('c', messages, my_num_[1] - '0', is_primary_);
            to_master_->SendMsg(REC_HASH);
            break;
          // Start data migration
          case MGR_STAR:
            if (CacheChange::GetStatus()[0] == 'e') {
            CacheChange::CacheExpand::StartExpand(cache_,to_master_, is_primary_);
            } else {
            CacheChange::CacheContract::StartContract(cache_,to_master_, is_primary_);
            }
            break;
          // Data migration finished
          case MGR_COMP:
           CacheChange::MigrationConfirmed(cache_);
            break;
          // Deleted cache to exit
          case CON_SHUT:
            return;
          default:
            break;
        }
      } else if (events[index].events&EPOLLERR
                 || events[index].events&EPOLLHUP
                 || events[index].events&EPOLLRDHUP) {

	        LogWarn << "The other side is closed!" <<std::endl;
          if ((*(std::unique_ptr<ServerSocket>*)events[index].data.ptr) == nullptr)
            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,from_standby_->sock->GetFD(),nullptr);
          else
            epoll_ctl(epoll_fd,
                      EPOLL_CTL_DEL,
                      (*(std::unique_ptr<ServerSocket>*)events[index].data.ptr)->sock->GetFD(),
                      nullptr);

      } else if ((*(std::unique_ptr<ServerSocket>*)events[index].data.ptr == nullptr)) {
		      LogError << "The other side is closed!" <<std::endl;
          epoll_ctl(epoll_fd,EPOLL_CTL_DEL,from_standby_->sock->GetFD(),nullptr);
      } else {
        std::string message = (*(std::unique_ptr<ServerSocket>*)events[index].data.ptr)->sock->RecvMsg();
        msg_t who_are_u = mt::GetMsgType(message);
        if (who_are_u == IAM_CLNT) {
           LogInfo << "Connect to client successfully." << std::endl;
        } else if ( who_are_u == IAM_CACH) {
          from_standby_ = std::move(*((std::unique_ptr<ServerSocket>*)events[index].data.ptr));
           LogInfo << "Connect to standby node successfully." << std::endl;
          // Start to migrate data.
          std::thread send_backup (&CacheManager::SendBackupData, this);
          send_backup.detach();
          // Request from clients
        } else if (who_are_u == PUT_DATA || who_are_u == GET_DATA){
          if (CacheChange::GetBusy())
            (*((std::unique_ptr<ServerSocket>*)events[index].data.ptr))->sock->SendMsg(LRU_BUSY);
          else
            GetRequest(*((std::unique_ptr<ServerSocket>*)events[index].data.ptr),message);
          }
          // Orders from write client to set the lru unbusy
        else if (who_are_u == SET_BUSY) {
            CacheChange::SetBusy(false);
            CacheChange::SetStatus({'n','n'});
        }
       }
     }
  }
}

/**
 * @brief Heartbeat send to Master
 * 
 */
void CacheManager::ReportHeartbeat() {
  while (true) {
    sleep(1);
    if (to_master_ == nullptr || to_master_->SendMsg(REP_HTBT, my_num_) <= 0)
      break;
  }
}


void CacheManager::GetRequest(const std::unique_ptr<ServerSocket>& from_client,
                              const std::string& message) {
  // Connection is created successfully.
  // Print the client information.
  LogInfo << "IP of client: "
            << inet_ntoa(from_client->addr->sin_addr) << ", "
            << "port: " << ntohs(from_client->addr->sin_port) << std::endl;
  // Communicate with client.

  std::string back_msg;
  if (!message.empty()) {
    // Make backup.
    if (from_standby_ != nullptr) {
      from_standby_->sock->SendMsg(message);
      back_msg = from_standby_->sock->RecvMsg();
      if (back_msg.empty() || mt::GetMsgType(back_msg) != BAC_SUCC) {
        LogWarn << "Make backup failed." << std::endl;
      }
    }
    switch (mt::GetMsgType(message)) {
      // Client wants to get data.
      case GET_DATA: {
        // Send the value of the key.
        from_client->sock->SendMsg(RET_VALE, this->GetData(mt::Get1stStr(message)));
        LogInfo << "Client get data." << std::endl;
        LogInfo << "    Key: " << mt::Get1stStr(message) << std::endl;
        cache_->PrintData();
        break;
        }
          // Client wants to store data.
      case PUT_DATA: {
        this->StoreData(mt::Get1stStr(message), mt::Get2ndStr(message));
        // Notify client data store success.
        from_client->sock->SendMsg(STO_SUCC);
        LogInfo << "Client store data." << std::endl;
        LogInfo << "   Key: " << mt::Get1stStr(message) << std::endl
                << "   Value: " << mt::Get2ndStr(message) << std::endl;
        cache_->PrintData();
        break;
        }
        default:
          break;
    }
  }
}

void CacheManager::GetBackupData() {
  // Connection is created successfully.
  // Communicate with primary node.
  std::string msg;
  while (true) {
    // This node has updated to primary.
    if (is_primary_)
      break;
    // Get backup data.
    msg = to_primary_->RecvMsg();
    if (!msg.empty()) {
      switch (mt::GetMsgType(msg)) {
        case GET_DATA:
          this->GetData(mt::Get1stStr(msg));
          cache_->PrintData();
          // Notify the primary node that the backup has succeed.
          if (to_primary_ != nullptr) {
          to_primary_->SendMsg(BAC_SUCC);
          }
          break;
        case PUT_DATA:
          this->StoreData(mt::Get1stStr(msg), mt::Get2ndStr(msg));
          cache_->PrintData();
          to_primary_->SendMsg(BAC_SUCC);
          break;
        case REC_DATA:
          this->HandleNewRequest(msg);
          break;
        default:
          break;
      }
    } else {
      break;
    }
  }
}

void CacheManager::SendBackupData() {
  // The child process send data to standby node.
  pid_t pid = fork();
  if (pid == 0) {
    auto data = cache_->Data();
    from_standby_->sock->SendMsg(REC_DATA, std::to_string(data.size()));
    for (auto it = data.crbegin(); it != data.crend(); ++it)
      from_standby_->sock->SendMsg(PUT_BACD, it->first, it->second);
  } else {
    // Waiting for recycle child process
    waitpid(pid, NULL, 0);
  }
}

void CacheManager::HandleNewRequest(std::string& msg) {
  // Get the count of data.
  int back_count = std::stoi(mt::Get1stStr(msg));
  // Store new data while recovering.
  std::queue<std::string> new_data;
  while (back_count > 0) {
    msg = to_primary_->RecvMsg();
    if (!msg.empty()) {
      switch (mt::GetMsgType(msg)) {
        case GET_DATA:
        case PUT_DATA:
          new_data.emplace(msg);
          break;
        case PUT_BACD:
          cache_->Store(mt::Get1stStr(msg), mt::Get2ndStr(msg));
          --back_count;
          break;
        default:
          break;
      }
    }
  }
  // Recover completed.
  while (!new_data.empty()) {
    // Handle data in queue for twice.
    // And handle new data from primary node for once.
    for (int i = 0; i < 2; i++) {
      msg = new_data.front();
      new_data.pop();
      if (mt::GetMsgType(msg) == GET_DATA)
        cache_->Get(mt::Get1stStr(msg));
      else
        cache_->Store(mt::Get1stStr(msg), mt::Get2ndStr(msg));
    }
    msg = to_primary_->RecvMsg();
    if (!msg.empty())
      new_data.emplace();
  }
}

int CacheManager::GetPort() {
  int my_num = std::stoi(my_num_);
  return ni_->MASTER_PORT + my_num * 100;
}