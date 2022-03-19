#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "CacheServer/Listener.h"
#include "Config/Logger.h"

Listener::Listener() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ == -1) {
    LogFatal << "Get socket";
    perror(nullptr);
  }
  // Set the SO_REUSEADDR option to reuse port.
  int opt_val;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val,
      sizeof(opt_val)) == -1) {
    LogFatal << "Set SO_REUSEADDR option";
    perror(nullptr);
  }
}

Listener::~Listener() {
    close(socket_fd_);
}

bool Listener::SetListen(unsigned short port) {
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  while (bind(socket_fd_, (sockaddr*)&saddr, sizeof(saddr)) == -1)
    usleep(500000);
  LogInfo << "Binding port succeeded." << std::endl;
  LogInfo << "  IP: " << inet_ntoa(saddr.sin_addr) << std::endl;
  LogInfo << "  Port: " << port << std::endl;

  int ret = listen(socket_fd_, 128);
  if (ret == -1) {
    LogFatal << "Listen on port";
    perror(nullptr);
    return false;
  }
  LogInfo << "Setting listen succeeded." << std::endl;
  return true;
}

std::unique_ptr<ActiveSocket> Listener::AcceptConn(sockaddr_in* addr) {
  socklen_t addrlen = sizeof(*addr);
  int from_client_fd = accept(socket_fd_, (sockaddr*)addr, &addrlen);
  if (from_client_fd == -1) {
      LogFatal << "Accept connection";
      perror(nullptr);
      return nullptr;
  }
  return std::make_unique<ActiveSocket>(from_client_fd);
}