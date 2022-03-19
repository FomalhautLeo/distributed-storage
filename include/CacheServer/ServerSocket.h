#ifndef STORAGE_CACHESERVER_SERVER_SOCKET_H_
#define STORAGE_CACHESERVER_SERVER_SOCKET_H_

#include <arpa/inet.h>
#include <memory>
#include <sys/socket.h>

#include "CacheServer/ActiveSocket.h"
#include "CacheServer/Listener.h"

struct ServerSocket {
  std::unique_ptr<ActiveSocket> sock;
  // Save information about IP and port.
  std::unique_ptr<sockaddr_in> addr;
  ServerSocket(std::unique_ptr<ActiveSocket>& sock, const sockaddr_in& addr);
};

#endif  // STORAGE_CACHESERVER_SERVER_SOCKET_H_