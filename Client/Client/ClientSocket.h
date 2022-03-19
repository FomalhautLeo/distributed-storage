#ifndef STORAGE_CLIENT_CLIENT_SOCKET_H_
#define STORAGE_CLIENT_CLIENT_SOCKET_H_

#include <string>
#include <unistd.h>

#include "Config/MessageType.h"

class ClientSocket {
 public:
  ClientSocket();
  //Create communication object for the server.
  ClientSocket(int socket);
  ~ClientSocket();
  bool ConnectToServer(const std::string& ip, unsigned short port, bool silence);
  int SendMsg(const std::string& msg);
  int SendMsg(msg_t msg_type);
  int SendMsg(msg_t msg_type,const std::string& param);
  int SendMsg(msg_t msg_type,const std::string& param1, const std::string& param2);
  std::string RecvMsg(bool flag = false);
  void Close() { close(socket_fd_); }

 private:
  // ClientSocket is neither copyable nor movable.
  ClientSocket(const ClientSocket& other) = delete;
  ClientSocket& operator=(const ClientSocket& other) = delete;

  int socket_fd_;
};

#endif  // STORAGE_CLIENT_CLIENT_SOCKET_H_