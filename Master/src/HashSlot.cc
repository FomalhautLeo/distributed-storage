#include "Master/HashSlot.h"
#include <iostream>
#include <algorithm>

#include "Config/Logger.h"

// Initialization of static members
std::vector<uint8_t> HashSlot::Crc_High_Table_(256,0);
std::vector<uint8_t> HashSlot::Crc_Low_Table_(256,0);


HashSlot::HashSlot() : slot_status_(hash_slot_size_,0) {}

// Copy Constructor.
// When changing the hash slot, the new hash slot is based on the old one(current one).
HashSlot::HashSlot(const HashSlot& hash_slot) : slot_status_(hash_slot.slot_status_),
    cache_load_status_(hash_slot.cache_load_status_) {}

/**
 * @brief Call it when initialize hash slot.
 * @param start_number How many caches are there at the beginning.
 */

void HashSlot::InitHashSlot(const unsigned short start_number) {
  if (start_number == 0) {
    LogFatal << "Error! At least one cache server needed!"<<std::endl;
    exit(1);
  }
  const std::unordered_map<unsigned short, CacheInfo> temp;
  ReDistribute(0, start_number,temp);
  return;
}

/**
 * @brief Call it when Master is trying to add a new cache.
 * This function will modified the hash slot
 * Some slots belong to old cache will be moved to new cache.
 */

void HashSlot::AddCache(const std::unordered_map<unsigned short, CacheInfo>& cache_list) {
  ReDistribute(1, CacheInfo::latest_cache_index_ + 1, cache_list);
  return;
}

/**
 * @brief Call it when Master is trying to delete an old cache or some caches are crush.
 * @param delete_cache_index the index of the cache that is planning to be delete or crush.
 * @param cache_list CacheInfo::cache_list, note that this is a const reference,
       the cache_list will be changed in DeleteCacheConfirmed().
 * This function will modified the hash slot
 * All of the slots belong to moving or crush cache will be moved all the other caches.
 */

void HashSlot::DeleteCache(const unsigned short delete_cache_index,
                           const std::unordered_map<unsigned short,
                           CacheInfo>& cache_list) {
  if (CacheInfo::alive_cache_number_ <= 1) {
    LogFatal<<"Error! 0 or 1 Alive Cache !"<<std::endl;
    exit(1);
  }
  ReDistribute(2, delete_cache_index, cache_list);
  return;
}

/**
 * @brief Call it when the Master confirmed all the caches initialized are online.
 *
 * @param start_number How many caches are there at the beginning.
 * @param cache_list CacheInfo::cache_list, only change it when confirm the successful operation.
 */

void HashSlot::InitHashSlotConfirmed(const unsigned short start_number,
                                     std::unordered_map<unsigned short,
                                     CacheInfo>& cache_list) {
  for (auto index = 0; index < start_number; ++index) {
    cache_list.emplace(index + 1,CacheInfo(true));
  }
  // Don't forget to change the CacheInfo::alive_cache_number_ and latest_cache_index in CashInfo.
  CacheInfo::SetAliveCacheNumber(start_number);
  CacheInfo::SetLatestCacheIndex(start_number);
}


/**
 * @brief Call it when Master confirm the add cache operation is finish.
 *
 * @param cache_list CacheInfo::cache_list, stores all the objects of CacheInfo
 */

void HashSlot::AddCacheConfirmed(std::unordered_map<unsigned short, CacheInfo>& cache_list) {
  CacheInfo::SetAliveCacheNumber(CacheInfo::GetAliveCacheNumber() + 1);
  CacheInfo::SetLatestCacheIndex(CacheInfo::GetLatestCacheIndex() + 1);
  cache_list.emplace(CacheInfo::GetLatestCacheIndex(), CacheInfo(true));
}


/**
 * @brief Call it when Master confirm the delete cache operation is successful.
 *
 * @param delete_cache_index the index of the cache that is planning to be delete or crush.
 * @param cache_list CacheInfo::cache_list, it will be changed here.
 */

void HashSlot::DeleteCacheConfirmed(const unsigned short delete_cache_index,
                                    std::unordered_map<unsigned short,
                                    CacheInfo>& cache_list) {
  cache_list.erase(delete_cache_index);

  CacheInfo::SetAliveCacheNumber(CacheInfo::GetAliveCacheNumber() - 1);
  CacheInfo::SetLatestCacheIndex(CacheInfo::GetLatestCacheIndex() -
      (CacheInfo::GetLatestCacheIndex() == delete_cache_index ? 1 : 0));
}



/**
 * @brief Generate two tables for accelerating the CRC16.
 */

void HashSlot::GenerateTableForCrc16() noexcept {
  uint16_t crc = 0;
  uint16_t outer_index,inner_index;
  for (outer_index = 0;outer_index<256;outer_index++) {
    crc = outer_index;
    for (inner_index = 0; inner_index < 8; inner_index++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
    Crc_High_Table_[outer_index] = (uint8_t)(crc&0xff);
    Crc_Low_Table_[outer_index] = (uint8_t)((crc>>8)&0xff);
    }
}

/**
 * @brief return (CRC16(random string) % 16384), decide which slot the random string belongs to.
 */

uint16_t HashSlot::Crc16(const std::string key, unsigned short len) noexcept {
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



void HashSlot::ReDistribute(unsigned short change_code, unsigned short change_index,
    const std::unordered_map<unsigned short, CacheInfo>& cache_list) {
  switch(change_code) {
    case 0 : {
      const auto future_load = hash_slot_size_ / change_index;
      auto remainder = hash_slot_size_ % change_index;
      auto more_quota = remainder-- > 0 ? 1 : 0;
      for (auto index = 1; index <= change_index; ++index) {
        cache_load_status_.emplace(index, future_load + more_quota);
        more_quota = remainder-- > 0 ? 1 : 0;
      }
      ReDistributeHashSlot(0, change_index,cache_load_status_);
    }
    break;
    case 1 : {
      const auto current_load = hash_slot_size_ / CacheInfo::alive_cache_number_;
      const auto future_load = hash_slot_size_ / (CacheInfo::alive_cache_number_ + 1);
      auto remainder = hash_slot_size_ % (CacheInfo::alive_cache_number_ + 1);
      std::vector<std::pair<short,unsigned short>> inequilibrium_rank;
      RankInequilibrium(current_load, inequilibrium_rank, cache_list);

      std::unordered_map<unsigned short, unsigned short> cache_quota;
      auto more_quota = remainder-- > 0 ? 1 : 0;
      unsigned short new_load = future_load + more_quota;
      for(const auto& val : inequilibrium_rank) {
        cache_quota.emplace(val.second,(cache_load_status_[val.second] - new_load));
        cache_load_status_[val.second] = new_load;
        more_quota = remainder-- > 0 ? 1 : 0;
        new_load = future_load + more_quota;
      }
      cache_load_status_.emplace(change_index, future_load);
      ReDistributeHashSlot(1, change_index, cache_quota);
    }
    break;
    case 2 : {
      const auto current_load = hash_slot_size_ / CacheInfo::alive_cache_number_;
      const auto future_load = hash_slot_size_ / (CacheInfo::alive_cache_number_ - 1);
      auto remainder = hash_slot_size_ % (CacheInfo::alive_cache_number_ - 1);
      std::vector<std::pair<short,unsigned short>> inequilibrium_rank;
      RankInequilibrium(current_load, inequilibrium_rank, cache_list,change_index);

      std::unordered_map<unsigned short, unsigned short> cache_quota;
      auto more_quota = remainder-- > 0 ? 1 : 0;
      unsigned short new_load;
      for(const auto& val : inequilibrium_rank) {
        new_load = future_load + more_quota;
        cache_quota.emplace(val.second,(new_load - cache_load_status_[val.second]));
        cache_load_status_[val.second] = future_load + more_quota;
        more_quota = remainder-- > 0 ? 1 : 0;
      }
      cache_load_status_.erase(change_index);
      ReDistributeHashSlot(2, change_index, cache_quota);
    }
    break;
    default : {}
    break;
    }

}



void HashSlot::RankInequilibrium(
    unsigned short current_load,
    std::vector<std::pair<short,unsigned short>>& inequilibrium_rank,
    const std::unordered_map<unsigned short, CacheInfo>& cache_list,
    unsigned short delete_cache_index /* = 0 */) {
  unsigned short cache_index;
  for (const auto& val : cache_list) {
    cache_index = val.first;
    if (cache_index != delete_cache_index && val.second.GetCacheStatus()) {
    inequilibrium_rank.emplace_back(
       std::pair<short, unsigned short>(static_cast<short>
           (static_cast<short>(current_load) - cache_load_status_[cache_index]),cache_index));
    }

  std::sort(inequilibrium_rank.begin(), inequilibrium_rank.end(),std::less<>());
  }
}


void HashSlot::ReDistributeHashSlot(unsigned short change_code, unsigned short change_index,
    std::unordered_map<unsigned short, unsigned short>& cache_quota) {
  switch(change_code) {
    case 0 : {
      auto cache_index = 1;
      auto cache_count = 0;
      auto this_quota = cache_quota[cache_index];
      for(auto& val : slot_status_) {
        if (cache_count == this_quota) {
          this_quota = cache_quota[++cache_index];
          cache_count = 0;
        }
        val = cache_index;
        ++cache_count;
      }
    }
    break;
    case 1 : {
      for(auto& val : slot_status_) {
        if (cache_quota[val] > 0) {
          --cache_quota[val];
          val = change_index;
        }
      }
    }
    break;
    case 2 : {
      auto expand_cacheS = std::vector<unsigned short>();
      for_each(cache_quota.begin(),cache_quota.end(),[&expand_cacheS](const auto& val) {
          expand_cacheS.emplace_back(val.first);});
      auto index = 0;
      for(auto& val : slot_status_) {
        if (val == change_index) {
          if(cache_quota[expand_cacheS[index]] == 0) {
            ++index;
          }
          --cache_quota[expand_cacheS[index]];
          val = expand_cacheS[index];
        }
      }
    }
    break;
    default : {}
    break;
  }
}
