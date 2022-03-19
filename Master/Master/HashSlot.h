#ifndef  STORAGE_MASTER_HASH_SLOT_H_
#define  STORAGE_MASTER_HASH_SLOT_H_
#include <cstdint>
#include <string>
#include <vector>
#include "Master/CacheInfo.h"

// This class controls the HashSlot of Master.
// This class holds one member:slot_status_, which is a vector of size 16384 represents the hash slot
// and the elements inside indicate which cache server each hash slot belongs to.
// The InitHashSlot(), AddCache(), DeleteCache() can change the slot_status_ and cache_load_status_
// when new caches are online or alive caches are crush or removed.
// Note that unlike CacheInfo, the slot_status_ and cache_load_status_ are not static
// for they will change everytime caches changes to be online or offline
// and one copy of predecess status must be backup for the possible failure
// when new caches are trying to online and etc.
// Three methods ends with "Confirmed" means that the CacheInfo::cache_list_ will only be changed
// after Master receives signals about operation success.
// CRC16 algorithm is used to calculated which slot one random string below to.

class HashSlot {
 public:
  explicit HashSlot();
  virtual ~HashSlot() = default;
  HashSlot(const HashSlot&);
  HashSlot& operator=(const HashSlot&) = delete;

  inline const std::vector<unsigned short>& GetSlotStatus() const {
    return this->slot_status_;
  }

  inline const std::unordered_map<unsigned short, unsigned short>& GetCacheLoadStatus() const {
    return this->cache_load_status_;
  }

  void InitHashSlot(const unsigned short);
  void InitHashSlotConfirmed(
      const unsigned short, std::unordered_map<unsigned short, CacheInfo>&);

  void AddCache(const std::unordered_map<unsigned short, CacheInfo>&);
  void AddCacheConfirmed(std::unordered_map<unsigned short, CacheInfo>&);

  void DeleteCache(
      const unsigned short, const std::unordered_map<unsigned short, CacheInfo>&);
  void DeleteCacheConfirmed(
      const unsigned short, std::unordered_map<unsigned short, CacheInfo>&);

  static void GenerateTableForCrc16() noexcept;
  static uint16_t Crc16(const std::string, unsigned short len) noexcept;

private:
  static constexpr unsigned short hash_slot_size_ = 16384;
  // Maintain who they belong to for every slot
  std::vector<unsigned short> slot_status_;
  // This is not the exact number of elements holds
  std::unordered_map<unsigned short, unsigned short> cache_load_status_;
  // for each cache! This is an auxiliary for AddCache() and DeleteCache().
  // This two table are created for accelerating the CRC16 algorithm.
  static std::vector<uint8_t> Crc_High_Table_;
  static std::vector<uint8_t> Crc_Low_Table_;
  void ReDistribute(unsigned short,
                    unsigned short,
                    const std::unordered_map<unsigned short,
                    CacheInfo>&);
  void RankInequilibrium(
      unsigned short, std::vector<std::pair<short,unsigned short>>&,
      const std::unordered_map<unsigned short, CacheInfo>&,
      unsigned short = 0);
  void ReDistributeHashSlot(unsigned short,
                            unsigned short,
                            std::unordered_map<unsigned short,
                            unsigned short>&);
};

#endif  // STORAGE_MASTER_HASH_SLOT_H_
