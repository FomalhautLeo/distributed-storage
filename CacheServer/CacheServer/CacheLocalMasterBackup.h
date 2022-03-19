#ifndef  STORAGE_CACHESERVER_CACHE_LOCAL_MASTER_BACKUP_H_
#define  STORAGE_CACHESERVER_CACHE_LOCAL_MASTER_BACKUP_H_

#include <string>
#include <vector>

// Store the current hash slot of the system.
// When the Master curshed, this class will serve as a backup for master.

namespace CacheLocalMasterBackup {
  // Update the slot_status if necessary.
  void UpdateHashSlot(const std::string&);
  void ConfirmPossibleHashSlot();

  std::vector<unsigned short>& GetHashSlot();

  const std::vector<unsigned short>& GetPossibleNewHashSlot();

  unsigned short GetCacheIndex(const std::string&, unsigned short);

  // Calculate the which cache one random string belongs to using local slot_status
  unsigned short Crc16(const std::string&, unsigned short len) noexcept;

  //See HashSlot::GenerateTableForCrc16
  void GenerateTableForCrc16() noexcept;

  constexpr unsigned short hash_slot_size_ = 16384;
  // slot_status_ in this class
  extern std::vector<unsigned short> slot_status_;
  extern std::vector<unsigned short> possible_new_slot_status_;
  // This two table are created for accelerating the CRC16 algorithm.
  extern std::vector<uint8_t> Crc_High_Table_;
  extern std::vector<uint8_t> Crc_Low_Table_;
};  // namespace CacheLocalMasterBackup

#endif  // STORAGE_CACHESERVER_CACHE_LOCAL_MASTER_BACKUP_H_