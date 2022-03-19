#ifndef SOTRAGE_CLIENT_CLIENT_MANAGER_H_
#define SOTRAGE_CLIENT_CLIENT_MANAGER_H_

#include <unordered_map>
#include <vector>

#include "Client/ClientSocket.h"
#include "Config/MessageType.h"
#include "Config/NetworkInfo.h"

// This Class Controls Client

class ClientManager {
 public:
  explicit ClientManager(int speed);
  ~ClientManager() = default;
  void Run();

 private:
  // ClientManager is neither copyable nor movable.
  ClientManager(const ClientManager&);
  ClientManager& operator=(const ClientManager&) = delete;
  void Initialize();
  // Handle requests of store and get.
  void RequestHandler();
  // User Interface.
  void ShowMenu();
  void StoreData();
  void GetData();
  void DealWithToDo();
  void ComWithMaster();
  void ComWithRead();
  void ComWithWrite();
  // One cacheserver crashed and switch to the other node.
  std::string SwitchCache(const std::string& cache_num);

  int times_;
  std::string hash_slot_;
  std::shared_ptr<NetworkInfo> ni_;
  std::unique_ptr<ClientSocket> to_master_;
  // <cache_num, socket>
  std::unordered_map<std::string, std::unique_ptr<ClientSocket>> to_caches_;
  // Record effective caches.
  std::unordered_map<char, std::string> eff_caches_;
  std::vector<std::string> keys_to_do_{};
  std::vector<std::string> values_to_do_{};
  static constexpr unsigned short size_of_k_ = 20;
  static constexpr unsigned short size_of_v_ = 200;
  static constexpr unsigned short fake_round_ = 10;

  bool change_busy_{false};
  bool hash_set_busy_{false};
  bool hash_get_busy_{false};
  bool use_new_{false};

  bool to_do_busy_{false};
  bool to_do_finish_{false};
  char select_ = '\0';
  char exp_or_con_ = '\0';
  std::string cache_index_string_ = "";
};

#endif  // SOTRAGE_CLIENT_CLIENT_MANAGER_H_