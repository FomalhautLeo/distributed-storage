#ifndef STORAGE_CACHESERVER_CACHE_MANAGER_H_
#define STORAGE_CACHESERVER_CACHE_MANAGER_H_

#include <atomic>
#include <memory>
#include <string>

#include "CacheServer/ActiveSocket.h"
#include "CacheServer/Listener.h"
#include "CacheServer/LRUCache.h"
#include "CacheServer/ServerSocket.h"
#include "Config/NetworkInfo.h"

class CacheManager {
 public:
  explicit CacheManager(const std::string& cache_num, int capacity);
  ~CacheManager();
  // Start to work.
  void Run();
  // Store data to cache.
  void StoreData(const std::string& key, const std::string& value) const;
  // Read data from cache.
  std::string GetData(const std::string&& key) const;
  int Capacity() const { return cache_->Capacity(); }
  int Size() const { return cache_-> Size(); }

 private:
  // CacheManager is neither copyable nor movable.
  CacheManager(const CacheManager& other) = delete;
  CacheManager& operator=(const CacheManager& other) = delete;

  int GetPort();
  void ConnectToMaster();
  // Standby node need to connect to primary node.
  void ConnectToPrimary();
  // Set listen from client and standby node.
  void Listen();
  // Report heart beat to master.
  void ReportHeartbeat();
  // From Client
  void GetRequest(const std::unique_ptr<ServerSocket>&, const std::string&);
  // From Primary Node
  void GetBackupData();
  // Send data to new standby node.
  void SendBackupData();
  // Handle new data request while recovering.
  void HandleNewRequest(std::string& msg);
  // Record the cache number.
  std::shared_ptr<NetworkInfo> ni_ = NetworkInfo::GetInstance();
  std::string my_num_;
  std::atomic_bool is_primary_;
  // Data is migrating and need to store temporary data.
  std::unique_ptr<LRUCache> cache_;
  std::unique_ptr<ActiveSocket> to_master_;
  std::unique_ptr<ActiveSocket> to_primary_;
  std::unique_ptr<ServerSocket> from_standby_;
};

#endif  // STORAGE_CACHESERVER_CACHE_MANAGER_H_