#include <cctype>

#include "Config/MessageType.h"

namespace mt {
msg_t GetMsgType(const std::string& message) {
  return message[0];
}

std::string Get1stStr(const std::string& message) {
  int count = 0;
  for (int i = 2; !isspace(message[i]); i++)
    count++;
  return message.substr(2, count);
}

std::string Get2ndStr(const std::string& message) {
  int count = 0;
  int i = 2;
  while (!isspace(message[i++])) {};
  for (int j = i; !isspace(message[j]); j++)
    count++;
  return message.substr(i, count);
}
}  // namespace mt