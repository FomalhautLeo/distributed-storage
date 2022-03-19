#ifndef STORAGE_CLIENT_CLIENT_LISTENER_H_
#define STORAGE_CLIENT_CLIENT_LISTENER_H_

#include <memory>
#include "Client/ClientSocket.h"

//

class ClientListener {
 public:
  explicit ClientListener();
  ~ClientListener();
  bool SetListen(unsigned short port);
  std::unique_ptr<ClientSocket> AcceptConn(struct sockaddr_in* addr);
  int GetFD() { return socket_fd_; }
  void Close() { close(socket_fd_); }

 private:
  // ClientListener is neither copyable nor movable.
  ClientListener& operator=(const ClientListener& other) = delete;
  ClientListener(const ClientListener& other) = delete;
  // Socket of listen
  int socket_fd_;
  unsigned short listen_port;
};

#endif  // STORAGE_CLIENT_CLIENT_LISTENER_H_