#ifndef STORAGE_CACHESERVER_ACTIVE_SOCKET_H_
#define STORAGE_CACHESERVER_ACTIVE_SOCKET_H_

#include <string>
#include <unistd.h>

#include "Config/MessageType.h"

// A class used to initiate active communication.
class ActiveSocket {
 public:
  ActiveSocket();
  // Create communication object for the server.
  ActiveSocket(int socket);
  ~ActiveSocket();
  bool ConnectToServer(const std::string& ip, unsigned short port);
  int SendMsg(const std::string& msg);
  int SendMsg(msg_t msg_type);
  int SendMsg(msg_t msg_type,const std::string& param);
  int SendMsg(msg_t msg_type,const std::string& param1, const std::string& param2);
  std::string RecvMsg(bool flag = false);

  int GetFD() { return socket_fd_; }
  void Close() { close(socket_fd_); }

 private:
  // ActiveSocket is neither copyable nor movable.
  ActiveSocket(const ActiveSocket& other) = delete;
  ActiveSocket& operator=(const ActiveSocket& other) = delete;
  int socket_fd_;
};

#endif  // STORAGE_CACHESERVER_ACTIVE_SOCKET_H_