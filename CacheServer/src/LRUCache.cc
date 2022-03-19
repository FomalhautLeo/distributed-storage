#include "CacheServer/LRUCache.h"
#include "Config/Logger.h"

LRUCache::LRUCache(int capacity) : capacity_(capacity), fixed_capacity_(capacity_) {
  lru_queue_.clear();
  hash_table_.clear();
}

LRUCache::~LRUCache() {

}

void LRUCache::Store(const std::string& key, const std::string& value) {
  busy_ = true;
  auto it = hash_table_.find(key);
  if (it != hash_table_.end()) {
    it->second->second = value;
    // Move the element to the head of lru_queue_.
    lru_queue_.splice(lru_queue_.begin(), lru_queue_, it->second);
  } else {
    if ((capacity_ != -1) && hash_table_.size() >= capacity_) {
      // If cache is already full, delete the oldest element.
      while (hash_table_.size() >= capacity_) {
        hash_table_.erase(lru_queue_.back().first);
        lru_queue_.pop_back();
      }
    }
    // Insert a new element.
    lru_queue_.emplace_front(key, value);
    hash_table_[key] = lru_queue_.begin();
  }
  busy_ = false;
}


std::string LRUCache::Get(const std::string& key) {
  busy_ = true;
  auto it = hash_table_.find(key);
  if (it == hash_table_.end()) {
    busy_ = false;
    return "NULL";
  }
  // Move the element to the head of lru_queue_.
  lru_queue_.splice(lru_queue_.begin(), lru_queue_, it->second);
  busy_ = false;
  return it->second->second;
}

void LRUCache::PrintData() {
  //for (auto kv : lru_queue_) {
    //std::cout << "{" << kv.first  << "} ";
    //std::cout << "{" << kv.first << "," << kv.second << "} ";
    LogDebug << "Size: " << hash_table_.size() << std::endl;
  //}
}