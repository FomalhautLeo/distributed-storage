#ifndef STORAGE_CACHESERVER_LISTENER_H_
#define STORAGE_CACHESERVER_LISTENER_H_

#include <memory>

#include "CacheServer/ActiveSocket.h"

class Listener {
 public:
  Listener();
  ~Listener();
  bool SetListen(unsigned short port);
  std::unique_ptr<ActiveSocket> AcceptConn(struct sockaddr_in* addr);

  int GetFD() { return socket_fd_; }
  void Close() { close(socket_fd_); }

 private:
  // Listener is neither copyable nor movable.
  Listener(const Listener& other) = delete;
  Listener& operator=(const Listener& other) = delete;
  // Socket of listen
  int socket_fd_;
  unsigned short listen_port;
};

#endif  // STORAGE_CACHESERVER_SERVER_SOCKET_H_