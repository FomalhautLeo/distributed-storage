#include <netinet/in.h>
#include <fcntl.h>
#include <linux/input.h>

#include <sstream>
#include <thread>
#include <unordered_set>

#include "Config/Logger.h"
#include "Master/MasterServer.h"


Server::Server() {
  std::string log_path = "MasterLog.log";
  ServerLog::SetLogPath(log_path.c_str());
  hashslot_ = std::make_unique<HashSlot>();
  ni_ = NetworkInfo::GetInstance();

  lfd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd_ == -1) {
    LogFatal << "Socket Error" << std::endl;
    perror(nullptr);
    exit(1);
  }
  int opt = 1;
  // port muliplex
  setsockopt(lfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  memset(&serv_addr_,0,sizeof(serv_addr_));
  serv_addr_.sin_family = AF_INET;
  serv_addr_.sin_port = htons(ni_->MASTER_PORT);
  serv_addr_.sin_addr.s_addr;
  // bind
  if (bind(lfd_, (struct sockaddr*)&serv_addr_, sizeof(serv_addr_)) == -1) {
    LogFatal << "Bind Error" << std::endl;
    perror(nullptr);
    exit(1);
  }
  LogInfo << "Bind Successfully" << std::endl;
  // listen
  if (listen(lfd_, 128) == -1) {
    LogFatal << "Listen Error" << std::endl;
    perror(nullptr);
    exit(1);
  }
  LogInfo << "Listen Successfully" << std::endl;
}

Server::~Server() {
  close(lfd_);
  close(keyboard_fd_);
}


//Add socket file to socket_map_ and epoll object
void Server::Accept(const int& curfd) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int cfd = accept(curfd, (struct sockaddr*)&client_addr, &client_addr_len);
  if (cfd == -1) {
    LogWarn << "Accept Error" << std::endl;
    //perror("Accept Error");
    return;
  }
  std::string socket_ip(inet_ntoa(client_addr.sin_addr));  // get socket IP
  const int socket_port(client_addr.sin_port);  // get socket port
  LogInfo<<socket_ip + " " + std::to_string(socket_port)
      + " new connection was accepted."<<std::endl;
  socket_map_.insert(make_pair(cfd,make_pair(socket_ip,socket_port)));

  ev_.events = EPOLLIN;  // set readable
  ev_.data.fd = cfd;
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, cfd, &ev_) == -1) {
    LogWarn << "Error:Epoll_ctl Accept" << std::endl;
    exit(1);
  }
}

void Server::Recv(const int& curfd) {
  std::string recv_message = recvMsg(curfd);
  int len = recv_message.size();
  static unsigned short add_p_fd;
  if (len == 0) {
    if (client_map_.find(curfd)!=client_map_.end()) {
      LogWarn << std::to_string(client_map_[curfd].second)+": Client has been offline."<<std::endl;
      client_map_.erase(curfd);
      socket_map_.erase(curfd);
      epoll_ctl(epfd_, EPOLL_CTL_DEL, curfd, NULL);
      close(curfd);
    }
  }
  else if (len > 0) {
    const msg_t msg_type = mt::GetMsgType(recv_message);
    const std::string key = mt::Get1stStr(recv_message);
    switch(msg_type) {
      case IAM_CACH:{
        LogInfo << "The cache "+key+" reached" << std::endl;
        unsigned short is_one_two = std::stoi(key.substr(0,1));  // zero or one
        unsigned short input_cache_index = std::stoi(key.substr(1));  // cache index
          if (is_one_two == 1 && one_cache_list_.find(input_cache_index) == one_cache_list_.end()) {
            // Record the file of the new cache during capacity expansion
            add_p_fd = curfd;
            // Notify primary node identity
            std::string reply_message(1,URE_PRIM);
            SendMsg(reply_message,curfd);
            SendHashslot(curfd);
            cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'p')));
          } else if (is_one_two == 1) {
             // The CacheNum of another Cache in the same group
             int flag = 2*10+input_cache_index;
             if (one_cache_list_.find(input_cache_index) != one_cache_list_.end()) {
               // The counter for heartbeat detection is set to 0
               one_cache_list_[input_cache_index].SetCacheCount(0);
               /* If another cache in the same group is not online, it is recognized
                  as the primary node, otherwise it is the standby node*/
               if (cache_.find(flag) == cache_.end()) {
                 std::string reply_message(1,URE_PRIM);
                 SendMsg(reply_message,curfd);
                 cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'p')));
               } else {
                 std::string reply_message(1,URE_STDB);
                 SendMsg(reply_message,curfd);
                 cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'s')));
               }
               one_cache_list_[input_cache_index].CacheInfo::SetCacheIp(socket_map_[curfd].first);
               one_cache_list_[input_cache_index].SetCachePort(socket_map_[curfd].second);
               // Send hash slot when cache goes online
               SendHashslot(curfd);
             }
          } else if (is_one_two == 2&&input_cache_index>CacheInfo::GetLatestCacheIndex()) {
             std::string reply_message(1,URE_STDB);
             SendMsg(reply_message,curfd);
             SendHashslot(curfd);
             CacheInfo two_cache(true,socket_map_[curfd].first,socket_map_[curfd].second,0);
             two_cache_list_.insert(std::make_pair(input_cache_index,two_cache));
             cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'s')));
             // Create a thread for capacity expansion
             std::thread tadd(&Server::AddCacheOperation,this,add_p_fd,curfd,input_cache_index);
             tadd.detach();
          } else if (is_one_two == 2) {
             int flag=1*10+input_cache_index;
             if (two_cache_list_.find(input_cache_index) != two_cache_list_.end()) {
               two_cache_list_[input_cache_index].SetCacheCount(0);
             }
             if (cache_.find(flag) == cache_.end()) {
               std::string reply_message(1,URE_PRIM);
               SendMsg(reply_message,curfd);
               cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'p')));
             } else {
               std::string reply_message(1,URE_STDB);
               SendMsg(reply_message,curfd);
               cache_.insert(std::make_pair(std::stoi(key),std::make_pair(curfd,'s')));
             }
             SendHashslot(curfd);
             CacheInfo two_cache(true,socket_map_[curfd].first,socket_map_[curfd].second,0);
             two_cache_list_.insert(std::make_pair(input_cache_index,two_cache));
          }
        }
        break;
      case IAM_CLNT:{
        client_map_.insert(make_pair(curfd,make_pair(socket_map_[curfd].first,
            socket_map_[curfd].second)));
        SendHashslot(curfd);
        break;
      }
      case REP_HTBT:{
        LogDebug << "Received heartbeat from cache "+key <<std::endl;
        unsigned short is_one_two = std::stoi(key.substr(0,1));  //0 or 1
        unsigned short input_cache_index = std::stoi(key.substr(1));    //cache index
        if (one_cache_list_.find(input_cache_index) != one_cache_list_.end()) {
          if (is_one_two==1) {
            one_cache_list_[input_cache_index].SetCacheCount(0);
          } else {
            two_cache_list_[input_cache_index].SetCacheCount(0);
          }
        }
        break;
      }
      case GET_HASH:{
        LogInfo << "Received hash-request from " << client_map_[curfd].second << std::endl;
        const int cache_index=slot_[hashslot_->Crc16(key,key.length())];
        std::string reply_message(1,RET_ADDR);
        reply_message+="\t"+cache_index;
        SendMsg(reply_message,curfd);
        break;
      }
      case REC_HASH:{
        // Received hash slot confirmation feedback
        ++get_hash_num_;
        break;
      }
      case MGR_COMP:{
        // Received data migration confirmation feedback
        ++mig_data_num_;
        break;
      }
      default:
        break;
     }
   }
   else {
     LogError << "Recv Error" << std::endl;
     exit(1);
   }
 }


// This function is used to detect Cache heartbeat
void Server::HeartHandler() {
  while(1) {
    auto it = cache_.begin();
    for( ; it != cache_.end(); ) {
      unsigned short first_bit = (it->first)/10;
      unsigned short second_bit = (it->first)%10;
      if (first_bit == 1&&one_cache_list_.find(second_bit) != one_cache_list_.end()) {
        unsigned short count=one_cache_list_[second_bit].GetCacheCount();
        if (count == 5) {
            LogWarn << "The cache " << it->first << " has been offline." << std::endl;
            //Inform the other node upgrade to primary
            if (cache_.find(2*10+second_bit)!=cache_.end() &&
                cache_[2*10+second_bit].second == 's') {
              int upgrade_to_primary=cache_[2*10+second_bit].first;
              std::string reply_message(1,UPG_PMRY);
              SendMsg(reply_message,upgrade_to_primary);
              cache_[2*10+second_bit].second = 'p';
            }
            int fd = (it->second).first;
            socket_map_.erase(fd);
            epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            cache_.erase(it++);
          } else if (count < 5 && count >= 0) {
            count+=1;
            one_cache_list_[second_bit].SetCacheCount(count);
            ++it;
          } else {
            ++it;
          }
      } else if (first_bit == 2&&two_cache_list_.find(second_bit)!=two_cache_list_.end()) {
        unsigned short count=two_cache_list_[second_bit].GetCacheCount();
        if (count == 5) {
          LogWarn << "The cache " << (it->first) << " has been offline." << std::endl;
          //Inform the other node upgrade to primary
          if (cache_.find(1*10+second_bit)!=cache_.end()&&cache_[1*10+second_bit].second=='s') {
            int upgrade_to_primary = cache_[1*10+second_bit].first;
            std::string reply_message(1,UPG_PMRY);
            SendMsg(reply_message,upgrade_to_primary);
            cache_[1*10+second_bit].second = 'p';
          }
          int fd = (it->second).first;
          socket_map_.erase(fd);
          epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
          close(fd);
          cache_.erase(it++);
        } else if (count < 5 && count >= 0) {
          count += 1;
          two_cache_list_[second_bit].SetCacheCount(count);
          ++it;
        } else {
          ++it;
        }
      }
    }
    usleep(400000);  // Timed for 400ms*5
  }
}

void Server::SendMsg(const std::string &msg,const int &fd) {
  std::string to_send = msg+"\n";
  // Requested memory space: data length + packet header 4 bytes (storage data length)
  char* data = new char[to_send.size() + 4];
  int bigLen = htonl(to_send.size());  // //Big end to small end
  memcpy(data, &bigLen, 4);
  memcpy(data + 4, to_send.data(),to_send.size());
  int ret = send(fd,data,to_send.size() + 4,MSG_DONTWAIT);
  if (ret == -1) {
    LogWarn << "Failed to send data." << std::endl;
  }
  delete[] data;
}

void Server::SendHashslot(const int &fd) {
  std::stringstream ss;
  for(auto i:slot_) {
    ss << i;
  }
  std::string send_hash(1,NEW_HASH);
  send_hash += "\t"+ss.str();
  SendMsg(send_hash,fd);
}

// Broadcast hashslots ,expansion or contraction messages to clients
void Server::BroadcastHashslotToClient(unsigned short cache_index, msg_t code) {
  std::stringstream ss;
  for(auto i:slot_) {
    ss << i;
  }
  std::string send_hash(1,code);
  send_hash += "\t"+ss.str()+"\t"+std::to_string(cache_index)+"\n";
  char* data = new char[send_hash.size() + 4];
  int bigLen = htonl(send_hash.size());
  memcpy(data, &bigLen, 4);
  memcpy(data + 4, send_hash.data(), send_hash.size());
  for(auto conn:client_map_) {
    int ret = send(conn.first,data,send_hash.size()+4,0);
  }
  delete[] data;
}
/*Send the successful operation of expanding and
  shrinking capacity to all clients and caches*/
void Server::BroadcastSuccess() {
  std::string send_succ(1,MGR_COMP);
  send_succ += "\n";
  char* data = new char[send_succ.size() + 4];
  int bigLen = htonl(send_succ.size());
  memcpy(data, &bigLen, 4);
  memcpy(data + 4, send_succ.data(), send_succ.size());
  for(auto conn:socket_map_) {
    send(conn.first,data,send_succ.size()+4,0);
  }
  delete[]data;
}

// Broadcast to cache group to start data migration
void Server::BroadcastMigrateDataToCache() {
  std::string mgr_star(1,MGR_STAR);
  mgr_star += "\n";
  char* data = new char[mgr_star.size() + 4];
  int bigLen = htonl(mgr_star.size());
  memcpy(data, &bigLen, 4);
  memcpy(data + 4, mgr_star.data(), mgr_star.size());
  for(auto i:cache_) {
    send(i.second.first,data,mgr_star.size()+4,0);
  }
  delete[] data;
}

int Server::readn(char* buf, int size,const int &fd) {
  int nread = 0;
  int left = size;;
  char* p = buf;
  while (left > 0) {
    if ((nread = recv(fd, p, left,MSG_DONTWAIT)) > 0) {
      p += nread;
      left -= nread;
    } else if (nread == -1) {
      return -1;
    } else if (nread==0) {
      return -1;
    }
  }
  return size;
}

std::string Server::recvMsg(const int &fd) {
  int len = 0;
  readn((char*)&len,4,fd);
  len = ntohl(len);

  // Allocate memory according to the read length
  char* buf = new char[len + 1];
  int ret = readn(buf, len,fd);
  if (ret != len) {
    return std::string();
  }
  buf[len] = '\0';
  std::string retStr(buf);
  delete[] buf;
  return retStr;
}

void Server::AddCacheOperation(const int &add_p_fd,const int &add_s_fd,
    const int &input_cache_index) {

  LogWarn << "Expansion start" << std::endl;
  // Save original hash slot
  std::vector<unsigned short>  old_slot=slot_;
  // Reassign hash slots
  hashslot_->AddCache(one_cache_list_);
  slot_=hashslot_->GetSlotStatus();
  hashslot_->AddCacheConfirmed(one_cache_list_);
  one_cache_list_[input_cache_index].SetCacheIp(socket_map_[add_p_fd].first);
  one_cache_list_[input_cache_index].SetCachePort(socket_map_[add_p_fd].second);

  get_hash_num_ = 0;

  std::stringstream ss;
  // send_con_new : messages sent to the new server
  std::string send_con_new(1,EXP_LSTN);
  for(auto i:slot_) {
    ss<<i;
  }
  send_con_new += "\t";
  send_con_new += ss.str() + "\t";
  // Number of old servers
  send_con_new += std::to_string(one_cache_list_.size()-1);
  // New server index
  send_con_new += std::to_string(input_cache_index) + std::to_string(input_cache_index);
  // sent to new primary node
  SendMsg(send_con_new,add_p_fd);
  // sent to new standby node
  SendMsg(send_con_new,add_s_fd);

  // send_con_old : messages sent to the old server
  std::string send_con_old = send_con_new;
  send_con_old += "-127.0.0.1:6000";
  // Assign the same port number to the primary and standby nodes
  std::unordered_map<unsigned short,unsigned short> count_map;
  unsigned short count = 0;
  for(auto i:cache_) {
    if (i.first%10!=input_cache_index) {
      if (count_map.find(i.first%10) == count_map.end()) {
        count_map[i.first%10] = count;
        std::string send_msg = send_con_old+std::to_string(count);
        SendMsg(send_msg,i.second.first);
        count++;
      } else {
        std::string send_msg = send_con_old+std::to_string(count_map[i.first%10]);
        SendMsg(send_msg,i.second.first);
      }
    }
  }
  //Send capacity expansion messages and hash slots to all clients
  BroadcastHashslotToClient(input_cache_index,EXP_LSTN);
  // Wait for hashslot confirmation
  sleep(1);
  if (get_hash_num_ == socket_map_.size())
    LogInfo << "Receive hashslot feedback successfully" << std::endl;
  else {
    LogWarn << "Receive hashslot feedback failed" << std::endl;
    return;
  }

  mig_data_num_ = 0;
  BroadcastMigrateDataToCache();
  // Wait for data migration to succeed
  sleep(1);
  bool flag=false;
  if (mig_data_num_ == 2) {
    flag=true;
    mig_data_num_ = 0;
  }

  if (flag) {
    LogInfo << "migrate data successfully"<<std::endl;
  } else {
    LogWarn << "migrate data failed"<<std::endl;
    slot_ = old_slot;
    return;
  }
  // Send a capacity expansion success notification to all caches of the client
  BroadcastSuccess();
  LogWarn << "addcache successfully" <<std::endl;
}


void Server::DeleteCacheOperation(const int &index) {
  LogWarn << "Contraction start" << index<<std::endl;
  std::vector<unsigned short> old_slot=slot_;
  hashslot_->DeleteCache(index,one_cache_list_);
  slot_ = hashslot_->GetSlotStatus();
  hashslot_->DeleteCacheConfirmed(index,one_cache_list_);

  // Tell the server that will not be deleted to set listening
  std::string not_delete_msg(1,CON_LSTN);
  std::stringstream ss;
  for(auto i:slot_) {
    ss << i;
  }
  not_delete_msg += "\t";
  not_delete_msg += ss.str() + "\t";
  not_delete_msg += std::to_string(one_cache_list_.size()+1);
  not_delete_msg += std::to_string(index);
  // Tell the server to be deleted the IP of all other servers
  std::string delete_msg = not_delete_msg;
  unsigned short delete_fd_p;
  unsigned short delete_fd_s;

  get_hash_num_ = 0;

  std::unordered_set<unsigned short> mask_set;
  for(auto i:cache_) {
    if (i.first%10 == index&&i.second.second =='p')
      delete_fd_p = i.second.first;
    else if (i.first%10 == index&&i.second.second == 's')
      delete_fd_s = i.second.first;
    else {
      if (mask_set.find(i.first%10) == mask_set.end()) {
        mask_set.insert(i.first % 10);
        delete_msg+=std::to_string(i.first%10)+"-127.0.0.1:6000"+std::to_string(i.first%10)+"+";
      }
      SendMsg(not_delete_msg,i.second.first);
    }
  }
  delete_msg.pop_back();
  SendMsg(delete_msg,delete_fd_p);
  SendMsg(delete_msg,delete_fd_s);
  BroadcastHashslotToClient(index,CON_LSTN);

  sleep(1);

  if (get_hash_num_ == socket_map_.size())
    LogInfo << "Receive hashslot feedback successfully" << std::endl;
  else {
    LogWarn << "Receive hashslot feedback failed" << std::endl;
    return;
  }

  mig_data_num_ = 0;
  BroadcastMigrateDataToCache();

  sleep(1);
  bool flag = false;
  if (mig_data_num_ == cache_.size() - 2) {
    flag = true;
    mig_data_num_ = 0;
  }
  if (flag) {
    LogInfo << "migrate data successfully" << std::endl;
  } else {
    LogWarn << "migrate data failed" << std::endl;
    slot_ = old_slot;
    return;
  }

  BroadcastSuccess();
  two_cache_list_.erase(index);
  cache_.erase(10+index);
  cache_.erase(20+index);
  socket_map_.erase(delete_fd_p);
  socket_map_.erase(delete_fd_s);
  epoll_ctl(epfd_, EPOLL_CTL_DEL, delete_fd_p, NULL);
  epoll_ctl(epfd_, EPOLL_CTL_DEL, delete_fd_s, NULL);

  // Tell the deleted cacheserver to shut down
  std::string send_shut(1,CON_SHUT);
  SendMsg(send_shut,delete_fd_p);
  SendMsg(send_shut,delete_fd_s);
  close(delete_fd_p);
  close(delete_fd_s);
}

void Server::Run(const int &start_number) {
  //init
  hashslot_->InitHashSlot(start_number);
  hashslot_->InitHashSlotConfirmed(start_number , one_cache_list_);
  HashSlot::GenerateTableForCrc16();
  slot_ = hashslot_->GetSlotStatus();
  get_hash_num_ = 0;
  mig_data_num_ = 0;

  //Create an epoll model,the input parameter must greater than 0
  epfd_ = epoll_create(200);
  if (epfd_ == -1) {
    LogFatal << "epoll_create Failed" <<std::endl;
    exit(1);
  }

  // Add lfd to epoll
  struct epoll_event ev_;
  ev_.events = EPOLLIN;
  ev_.data.fd = lfd_;

  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, lfd_, &ev_)== -1) {
    LogFatal << "Error:epoll_ctl_add lfd"<<std::endl;
    exit(1);
  }
  // open keyboard file
  keyboard_fd_=open("/dev/input/event1",O_RDONLY);

  struct epoll_event keyboard_ev_;
  keyboard_ev_.events=EPOLLIN;
  keyboard_ev_.data.fd=keyboard_fd_;

  struct input_event event;

  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, keyboard_fd_, &keyboard_ev_)== -1) {
    LogFatal << "Error:epoll_ctl_add keyboard_fd_" << std::endl;
    exit(1);
  }

  struct epoll_event evs[128];
  int size = sizeof(evs) / sizeof(struct epoll_event);
  // Create thread for heartbeat detect
  std::thread t(&Server::HeartHandler,this);
  int delete_index=0;
  bool delete_flag=false;

  while(true) {
    int num = epoll_wait(epfd_, evs, size, -1);
    for(int i=0; i<num; ++i) {
      int curfd = evs[i].data.fd;
      if (curfd == lfd_) {
        Accept(curfd);
      } else if (curfd == keyboard_fd_) {
        if (read(keyboard_fd_,&event,sizeof(event))) {
          if (event.type == EV_KEY&&event.value == 0) {
            if (delete_flag==true) {
              delete_index=event.code-1;
              delete_flag=false;
              std::thread tdelete(&Server::DeleteCacheOperation,this,delete_index);
              tdelete.detach();
            }
            if (event.code==1) {
              delete_flag=true;
            }
          }
        }
      } else {
        Recv(curfd);
      }
    }
  }
}
