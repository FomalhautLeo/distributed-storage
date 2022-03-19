#ifndef  STORAGE_CACHESERVER_CACHE_CHANGE_H_
#define  STORAGE_CACHESERVER_CACHE_CHANGE_H_

#include <future>
#include <memory>
#include <string>
#include <vector>

#include "CacheServer/ActiveSocket.h"
#include "CacheServer/Listener.h"
#include "CacheServer/LRUCache.h"

// Functions included in CacheServer Expand and Contract
namespace CacheChange {
  void PrepareChange(char, const std::string&, int, bool);

  bool GetBusy();
  void SetBusy(bool );

  std::string GetStatus();
  void SetStatus(const std::string&);

  void SetListenS(const unsigned short&, const bool&);
  const std::vector<std::unique_ptr<Listener> >& GetListenArray();

  const std::string& GetIpToSend();
  const std::string& GetIpSToSend();
  std::vector<std::string>& GetKeysToDelete();

  void InfoSplit(const std::string&, unsigned short&, std::string&,
                 unsigned short&, const char, const char, const char);

  void SendToCache(const std::unique_ptr<LRUCache>&,
                   std::vector<std::string>&,
                   const std::string&,
                   bool);
  std::string ReceiveFromCache(const std::unique_ptr<Listener>&,
                               const std::unique_ptr<LRUCache>&,
                               std::promise<char>&);
  void MigrationConfirmed(const std::unique_ptr<LRUCache>&);

  constexpr unsigned short size_of_k_ = 20;
  constexpr unsigned short size_of_v_ = 200;
  constexpr unsigned short size_of_kv_ = size_of_k_ + size_of_v_;
  constexpr unsigned short package_per_MTU_ = 1500 / size_of_kv_;
  constexpr unsigned short size_per_MTU_ = size_of_kv_ * package_per_MTU_;
  constexpr unsigned short change_listen_port_ = 60000;
  
  //Indicate whether CacheServer is write or read LRUCache for Cache Expand / Contract
  extern bool busy_;
  //State Machine for Cache Expand / Contract
  extern std::string status_ ;
  extern std::vector<std::unique_ptr<Listener> > listen_array_;
  extern std::string ip_to_send_;
  extern std::string ips_to_send_;
  // Store the sent data in case of Cache Expand fail
  extern std::vector<std::string> keys_to_delete_;
  extern unsigned short this_index_;

  namespace CacheExpand {
    void StartExpand(const std::unique_ptr<LRUCache>&,
                     const std::unique_ptr<ActiveSocket>&,
                     bool);
    void ReceiveFromOldS(const std::unique_ptr<LRUCache>&,
                         const std::unique_ptr<ActiveSocket>&);
  };  // namespace CacheExpand

  namespace CacheContract {
    void StartContract(const std::unique_ptr<LRUCache>&,
                       const std::unique_ptr<ActiveSocket>&,
                       bool);
    void ReceiveFromDying(
        const std::unique_ptr<LRUCache>&, const std::unique_ptr<ActiveSocket>&);
    void SendToAliveCacheS(const std::unique_ptr<LRUCache>&, bool);
    void StringSplit(const std::string&,std::vector<std::string>&, const char);
    // Used in deleted cacheserver during Cache Expand. store all the data in category in CacheNum
    extern std::unordered_map<unsigned short, std::string> multiple_data_to_send_;
  };  // namespace CacheContract

};  // namespace CacheChange

#endif  // STORAGE_CACHESERVER_CACHE_CHANGE_H_