#ifndef STORAGE_CLIENT_TEST_DATA_GENERATION_H_
#define STORAGE_CLIENT_TEST_DATA_GENERATION_H_

#include <random>
#include <string>
#include <vector>

namespace TestDataGeneration {
  void Initialize();
  // Get randomly generated letters
  void GetNum(std::string &str);

  extern std::vector<char> random_table_;
  extern std::random_device rd_;
  extern std::default_random_engine e_;
  extern std::uniform_int_distribution<unsigned> u_;
};

#endif  // !STORAGE_TEST_TEST_TEST_DATA_GENERATION_H_