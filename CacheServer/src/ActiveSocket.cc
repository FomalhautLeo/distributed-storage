#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

#include "CacheServer/ActiveSocket.h"
#include "Config/Logger.h"

// The length of the message is at the head of the message.
const int kHeadLength = 4;

ActiveSocket::ActiveSocket() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

ActiveSocket::ActiveSocket(int socket) : socket_fd_(socket) {

}

ActiveSocket::~ActiveSocket() {
  close(socket_fd_);
}

bool ActiveSocket::ConnectToServer(const std::string& ip, unsigned short port) {
  //std::cout << "IP: " << ip << std::endl;
  //std::cout << "Port: " << port << std::endl;
  sockaddr_in sock_addr;
  // Convert the information of server.
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);
  sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  int ret = connect(socket_fd_, (sockaddr*)&sock_addr, sizeof(sock_addr));
  if (ret == -1) {
    LogFatal << "Failed to connect to server" << std::endl;
    //perror(nullptr);
    return false;
  }
  return true;
}

int ActiveSocket::SendMsg(const std::string& msg) {
  // Request memory space: length of data + Packet header 4bytes(store length)
  int sz = msg.size();
  char data[sz + kHeadLength];
  int msg_len = htonl(sz);
  // Write the length of the message.
  memcpy(data, &msg_len, kHeadLength);
  memcpy(data + kHeadLength, msg.data(), msg.size());
  // Send data to socket.
  int ret = send(socket_fd_, data, sz + kHeadLength, MSG_NOSIGNAL);
  return ret;
}

int ActiveSocket::SendMsg(const msg_t msg_type){
  std::string str_msg(1, msg_type);
  str_msg += "\n";
  return SendMsg(str_msg);
}

int ActiveSocket::SendMsg(const msg_t msg_type, const std::string& str_param){
  std::string str_msg(1, msg_type);
  str_msg += "\t" + str_param +"\n";
  return SendMsg(str_msg);
}

int ActiveSocket::SendMsg(
    msg_t msg_type,const std::string& param1,const std::string& param2){
  std::string str_msg(1, msg_type);
  str_msg += "\t" + param1 + "\t" + param2 +"\n";
  return SendMsg(str_msg);
}

std::string ActiveSocket::RecvMsg(bool flag) {
  // The option of non block.
  int option = 0;
  if (flag)
    option = MSG_DONTWAIT;
  // Read packet header.
  int len = 0;
  recv(socket_fd_, &len, kHeadLength, option);
  len = ntohl(len);
  // Allocate memory according to the length.
  char buf[len + 1];
  int ret = recv(socket_fd_, buf, len, option | MSG_WAITALL);
  // If there's no message to receive, return a empty string.
  if (ret == -1 && errno == EAGAIN)
    return {};
  buf[len] = '\0';
  std::string ret_str(buf);
  return ret_str;
}