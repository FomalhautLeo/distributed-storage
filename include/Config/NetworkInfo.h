#ifndef STORAGE_CONFIG_NETWORK_INFO_H_
#define STORAGE_CONFIG_NETWORK_INFO_H_

#include <memory>
#include <string>
#include <unordered_map>

class NetworkInfo {
 public:
  // Get hte singleton instance.
  static std::shared_ptr<NetworkInfo> GetInstance();
  // IP and port numbers
  // Master
  const std::string MASTER_IP = "127.0.0.1";
  const unsigned short MASTER_PORT = 50000;
  // CacheServer
  const std::unordered_map<std::string, std::string> CACHE_IP {
    {"11", "127.0.0.1"},
    {"21", "127.0.0.1"},
    {"12", "127.0.0.1"},
    {"22", "127.0.0.1"},
    {"13", "127.0.0.1"},
    {"23", "127.0.0.1"},
    {"14", "127.0.0.1"},
    {"24", "127.0.0.1"},
    {"15", "127.0.0.1"},
    {"25", "127.0.0.1"},
    {"16", "127.0.0.1"},
    {"26", "127.0.0.1"}
  };
  const std::unordered_map<std::string, unsigned short> CACHE_PORT {
    {"11", 51100},
    {"21", 52100},
    {"12", 51200},
    {"22", 52200},
    {"13", 51300},
    {"23", 52300},
    {"14", 51400},
    {"24", 52400},
    {"15", 51500},
    {"25", 52500},
    {"16", 51600},
    {"26", 52600}
  };
  const std::string CLIENT_IP = "127.0.0.1";
  const unsigned short CLIENT_PORT = 55000;
 private:
  NetworkInfo() = default;
  NetworkInfo(const NetworkInfo& other) = delete;
  NetworkInfo& operator=(const NetworkInfo& other) = delete;
  static std::shared_ptr<NetworkInfo> instance_;
};

#endif  // STORAGE_CONFIG_NETWORK_INFO_H_