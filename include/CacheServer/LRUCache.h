#ifndef STORAGE_CACHESERVER_LRUCACHE_H_
#define STORAGE_CACHESERVER_LRUCACHE_H_

#include <list>
#include <string>
#include <unordered_map>

#include "Config/MessageType.h"

// Implement LRU mechanism.
class LRUCache {
 public:
   // key-value type
  typedef std::pair<std::string, std::string> kv_t;

  explicit LRUCache(int capacity);
  ~LRUCache();

  void Store(const std::string& key, const std::string& value);
  std::string Get(const std::string& key);
  int Capacity() const { return capacity_; }
  int Size() const { return hash_table_.size(); }
  void PrintData();
  void LockCapacity() { capacity_ = -1; }

  void UnlockCapacity() { capacity_ = fixed_capacity_; }
  bool GetBusy() const { return busy_; }

  const std::list<kv_t>& Data() const { return lru_queue_; }
  std::list<kv_t>& GetList() { return lru_queue_; }
  std::unordered_map<std::string, std::list<kv_t>::iterator>& GetTable() {
    return hash_table_;
  }

 private:
  // LRUCache is neither copyable nor movable.
  LRUCache(const LRUCache& other) = delete;
  LRUCache& operator=(const LRUCache& other) = delete;

  bool busy_;
  int capacity_;
  int fixed_capacity_;
  // Record the sequence of LRU.
  std::list<kv_t> lru_queue_;
  // Save key-value data.
  // hash_table_[num].first:  key
  // hash_table_[num].second: value
  std::unordered_map<std::string, std::list<kv_t>::iterator> hash_table_;
};

#endif  // SOTRAGE_CACHESERVER_LRUCACHE_H_