#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Client/ClientListener.h"
#include "Config/Logger.h"

ClientListener::ClientListener() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ == -1) {
    LogFatal << "Failed to get socket." << std::endl;
    //perror(nullptr);
  }
  // Set the option to reuse port.
  int opt_val;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val,
      sizeof(opt_val)) == -1) {
    LogFatal << "Failed to set SO_REUSEADDR option." << std::endl;;
    //perror(nullptr);
  }
}

ClientListener::~ClientListener() {
  close(socket_fd_);
}

bool ClientListener::SetListen(unsigned short port) {
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
    LogFatal << "Filed to listen on port." << port << std::endl;;
    //perror(nullptr);
    return false;
  }
  LogInfo << "Setting listen succeeded." << std::endl;
  return true;
}

std::unique_ptr<ClientSocket> ClientListener::AcceptConn(sockaddr_in* addr) {
  socklen_t addrlen = sizeof(*addr);
  int from_client_fd = accept(socket_fd_, (sockaddr*)addr, &addrlen);
  if (from_client_fd == -1) {
    LogFatal << "Failed to accept connection." << std::endl;;
    //perror(nullptr);
    return nullptr;
  }
  return std::make_unique<ClientSocket>(from_client_fd);
}