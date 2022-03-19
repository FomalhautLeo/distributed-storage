#ifndef  STORAGE_CLIENT_CLIENT_LOCAL_MASTER_BACKUP_H_
#define  STORAGE_CLIENT_CLIENT_LOCAL_MASTER_BACKUP_H_

#include <string>
#include <vector>

// It will store the current hash slot of the system.
// When the Master crush, this class will serve as a backup for master.

namespace ClientLocalMasterBackup {
  // Update the slot_status if necessary
  void UpdateHashSlot(const std::string&);
  void ConfirmPossibleHashSlot();
  std::vector<unsigned short>& GetHashSlot();
  const std::vector<unsigned short>& GetPossibleNewHashSlot();
  // Calculate the which cache one random string belongs to using local slot_status
  unsigned short Crc16(const std::string&, unsigned short len) noexcept;
  unsigned short GetCacheIndex(const std::string&, unsigned short, bool);
  //See HashSlot::GenerateTableForCrc16
  void GenerateTableForCrc16() noexcept;
  constexpr unsigned short hash_slot_size_ = 16384;
  // slot_status_ in this namespace
  extern std::vector<unsigned short> slot_status_;
  extern std::vector<unsigned short> possible_new_slot_status_;
  // This two table are created for accelerating the CRC16 algorithm.
  extern std::vector<uint8_t> Crc_High_Table_;
  extern std::vector<uint8_t> Crc_Low_Table_;
};  // namespace ClientLocalMasterBackup

#endif  // STORAGE_CLIENT_CLIENT_LOCAL_MASTER_BACKUP_H_