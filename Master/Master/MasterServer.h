#ifndef STORAGE_MASTER_MASTERSERVER_H_
#define STORAGE_MASTER_MASTERSERVER_H_

#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <unordered_map>

#include "Config/MessageType.h"
#include "Config/NetworkInfo.h"
#include "Master/HashSlot.h"

class Server{
public:
  Server();
  ~Server();
  void Run(const int&);

 private:
  int lfd_;  // Socket file
  int keyboard_fd_;  // Keyboard file
  int epfd_;  // Epoll object
  struct epoll_event ev_;
  struct sockaddr_in serv_addr_;
  std::vector<unsigned short> slot_;  // Hashslot
  std::unique_ptr<HashSlot> hashslot_;
  // Record all connections <fd,<ip, port>>
  std::unordered_map<int, std::pair<std::string, int> > socket_map_;
  // Record clients connected  <fd,<ip, port>>
  std::unordered_map<int, std::pair<std::string, int> > client_map_;
  std::unordered_map<unsigned short, CacheInfo> one_cache_list_;  //11 12 13 ....
  std::unordered_map<unsigned short, CacheInfo> two_cache_list_;   //21 22 23 ....
  // Include all caches <index,<fd,p?s>>
  std::unordered_map<int,std::pair<int,char>> cache_;
  std::shared_ptr<NetworkInfo> ni_;
  // Record the number of hash slot confirmation feedback during expansion and contraction
  std::atomic_ushort get_hash_num_;
  // Record the number of data migration confirmation feedback during capacity expansion and contraction
  std::atomic_ushort mig_data_num_;

  void Accept(const int&);
  void Recv(const int&);
  void SendMsg(const std::string& , const int&);
  int readn(char* , int , const int&);
  std::string recvMsg(const int&);

  void HeartHandler();
  void BroadcastHashslotToClient(unsigned short, msg_t);
  void BroadcastMigrateDataToCache();
  void SendHashslot(const int&);
  void BroadcastSuccess();
  void AddCacheOperation(const int& , const int& , const int&);
  void DeleteCacheOperation(const int&);
};
#endif  // STORAGE_MASTER_MASTERSERVER_H_