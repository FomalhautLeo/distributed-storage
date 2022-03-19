#ifndef  STORAGE_MASTER_CACHE_INFO_H_
#define  STORAGE_MASTER_CACHE_INFO_H_

#include <string>
#include <unordered_map>

// For each cache server online, one of this object will be instanced and stores the
// index,status... of the cache server.
// Static member stores the statistic data of all the objects of this class.
class CacheInfo {
  friend class HashSlot;
 public:
  CacheInfo()=default;
  explicit CacheInfo(bool);
  CacheInfo(bool,std::string,int,unsigned short);
  virtual ~CacheInfo() = default;
  CacheInfo& operator=(const CacheInfo&) = delete;
  CacheInfo(const CacheInfo&);
  // Several static method to get and set the statistic data
  static unsigned short GetAliveCacheNumber();
  static void SetAliveCacheNumber(unsigned short);
  static unsigned short GetDeadCacheNumber();
  static void SetDeadCacheNumber(unsigned short);
  static unsigned short GetLatestCacheIndex();
  static void SetLatestCacheIndex(unsigned short);

  std::string GetCacheIp() const;
  void SetCacheIp(std::string);
  int GetCachePort() const;
  void SetCachePort(int param);
  unsigned short GetCacheCount() const;
  void SetCacheCount(unsigned short);

  bool GetCacheStatus() const;
  void SetCacheStatus(bool);

 private:
  static unsigned short alive_cache_number_;  // number of online caches
  static unsigned short dead_cache_number_;   // number of offline caches
  static unsigned short latest_cache_index_;  // largest index of cache by far

  bool cache_status_ = true;                  // status of this cache, online = true, offline = false;
  std::string cache_ip_= "";                  //ip adress of this cache
  int cache_port_=0;                          // ip port of this cache
  unsigned short cache_count_=0;              // used for heartbeat check
};

#endif  // STORAGE_MASTER_CACHE_INFO_H_