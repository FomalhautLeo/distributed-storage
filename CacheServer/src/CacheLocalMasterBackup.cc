#include "CacheServer/CacheLocalMasterBackup.h"
#include "Config/Logger.h"

std::vector<unsigned short> CacheLocalMasterBackup::slot_status_; // slot_status_ in this class
std::vector<unsigned short> CacheLocalMasterBackup::possible_new_slot_status_{};
std::vector<uint8_t> CacheLocalMasterBackup::Crc_High_Table_(256,0);     // This two table are created for accelerating the CRC16 algorithm.
std::vector<uint8_t> CacheLocalMasterBackup::Crc_Low_Table_(256,0);


/**
 * @brief This Method can update the Local HashSlot in Client by orders from Master
 * 
 * @param received_string message receive from Master, contains New hashslot
 */

void CacheLocalMasterBackup::UpdateHashSlot(const std::string& received_string) {
  if (slot_status_.size() == 0) {
    slot_status_.resize(hash_slot_size_);
    auto index = 0;
    for (const auto& val : received_string) {
    slot_status_[index++] = static_cast<unsigned short>(val - '0');
    }
    LogInfo << "Receive the first hash slot successfully." << std::endl;
    return;
  }
  if (possible_new_slot_status_.size() > 0){
    possible_new_slot_status_.clear();
  }
  for (const auto& val : received_string) {
    possible_new_slot_status_.emplace_back(static_cast<unsigned short>(val - '0'));
  }
}

void CacheLocalMasterBackup::ConfirmPossibleHashSlot() {
  slot_status_ = std::move(possible_new_slot_status_);
}

/**
 * @brief return (CRC16(random string) % 16384)
 *   decide which slot the random string belongs to.
 */
unsigned short CacheLocalMasterBackup::Crc16(const std::string& key,
                                             unsigned short len) noexcept {
  const auto klen = len;
  uint8_t crchi = 0x00;
  uint8_t crclo = 0x00;
  uint8_t index = 0;
  uint16_t crc = 0;
  for (;len>0;len--) {
    index = crclo ^ key[klen - len];
    crclo = crchi ^ (Crc_High_Table_[index]);
    crchi = (Crc_Low_Table_[index]);
  }
  crc = (uint16_t)(crchi<<8 | crclo);
  return((crc^0xffff)&(static_cast<uint16_t>(16383)));
}

void CacheLocalMasterBackup::GenerateTableForCrc16() noexcept {
  uint16_t crc = 0;
  uint16_t outer_index,inner_index;
  for (outer_index = 0;outer_index<256;outer_index++) {
    crc = outer_index;
    for (inner_index = 0; inner_index < 8; inner_index++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
    Crc_High_Table_[outer_index] = (uint8_t)(crc&0xff);
    Crc_Low_Table_[outer_index] = (uint8_t)((crc>>8)&0xff);
  }
}

const std::vector<unsigned short>& CacheLocalMasterBackup::GetPossibleNewHashSlot() {
  return possible_new_slot_status_;
}

unsigned short CacheLocalMasterBackup::GetCacheIndex(const std::string& key,
                                                     unsigned short len) {
  return possible_new_slot_status_[Crc16(key,len)];
}
