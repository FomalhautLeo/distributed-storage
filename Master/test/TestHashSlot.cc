#include <gtest/gtest.h>
#include "Master/HashSlot.h"

//Unit Test For HashSlot

class HashSlotTest : public ::testing::Test{
  public:
    static std::vector<unsigned short> InitHashSlot_Test(unsigned short param) {
      cache_list_test_.clear();
      std::vector<unsigned short> ret;
      ret.resize(param);
      hashslot_test_->InitHashSlot(param);
      hashslot_test_->InitHashSlotConfirmed(param,cache_list_test_);
      for(const auto& val : cache_list_test_) {
        ret[val.first - 1] = hashslot_test_->GetCacheLoadStatus().at(val.first);
      }
      return ret;
    }

    static std::vector<unsigned short> AddCache_Test(unsigned short param) {
      cache_list_test_.clear();
      std::vector<unsigned short> ret;
      ret.resize(param + 1);
      hashslot_test_->InitHashSlot(param);
      hashslot_test_->InitHashSlotConfirmed(param,cache_list_test_);
      hashslot_test_->AddCache(cache_list_test_);
      hashslot_test_->AddCacheConfirmed(cache_list_test_);
      for(const auto& val : cache_list_test_) {
        ret[val.first - 1] = hashslot_test_->GetCacheLoadStatus().at(val.first);
      }
      return ret; 
    }

    static std::vector<unsigned short> DeleteCache_Test(unsigned short init_num, unsigned short delete_index) {
      cache_list_test_.clear();
      std::vector<unsigned short> ret;
      ret.resize(init_num,0);
      hashslot_test_->InitHashSlot(init_num);
      hashslot_test_->InitHashSlotConfirmed(init_num,cache_list_test_);
      hashslot_test_->DeleteCache(1,cache_list_test_);
      hashslot_test_->DeleteCacheConfirmed(1,cache_list_test_);
      for(const auto& val : cache_list_test_) {
        ret[val.first - 1] = hashslot_test_->GetCacheLoadStatus().at(val.first);
      }
      return ret;
    }
  
  private:
    static std::unique_ptr<HashSlot> hashslot_test_;
    static std::unordered_map<unsigned short, CacheInfo>& cache_list_test_;
};

TEST(InitHashSlot,InitHashSlot_In_Test) {
  std::vector<unsigned short> expect_ret_1{5462,5461,5461};
  EXPECT_EQ(expect_ret_1,HashSlotTest::InitHashSlot_Test(3));
  std::vector<unsigned short> expect_ret_2{4096,4096,4096,4096};
  EXPECT_EQ(expect_ret_1,HashSlotTest::InitHashSlot_Test(4));
}

TEST(AddCache,AddCache_In_Test) {
  std::vector<unsigned short> expect_ret_1{4096,4096,4096,4096};
  EXPECT_EQ(expect_ret_1,HashSlotTest::AddCache_Test(3));
  std::vector<unsigned short> expect_ret_2{3277,3277,3277,3277,3276};
  EXPECT_EQ(expect_ret_1,HashSlotTest::AddCache_Test(4));
}

TEST(DeleteCache,DeleteCache_In_Test) {
  std::vector<unsigned short> expect_ret_1{0,8192,8192};
  EXPECT_EQ(expect_ret_1,HashSlotTest::DeleteCache_Test(3,1));
  std::vector<unsigned short> expect_ret_2{5462,0,5461,5461};
  EXPECT_EQ(expect_ret_1,HashSlotTest::DeleteCache_Test(4,2));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
