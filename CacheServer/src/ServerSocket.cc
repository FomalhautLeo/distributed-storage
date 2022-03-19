#include "CacheServer/ServerSocket.h"

ServerSocket::ServerSocket(
    std::unique_ptr<ActiveSocket>& sock, const sockaddr_in& addr) {
  this->sock = std::move(sock);
  this->addr = std::make_unique<sockaddr_in>(addr);
}