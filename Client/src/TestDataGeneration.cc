#include "Client/TestDataGeneration.h"

std::vector<char> TestDataGeneration::random_table_ =
  {'0','1','2','3','4','5','6','7','8','9',
  'a','b','c','d','e','f','g','h','i','j','k','l','m',
  'n','o','p','q','r','s','t','u','v','w','x','y','z',
  'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '*','!','?','#','$','%','&','@'
};

std::random_device TestDataGeneration::rd_;
std::default_random_engine TestDataGeneration::e_;
std::uniform_int_distribution<unsigned> TestDataGeneration::u_(0,69);

void TestDataGeneration::Initialize() {
  e_.seed(rd_());
}

void TestDataGeneration::GetNum(std::string &str) {
  int len = str.size();
  for (int i = 0; i < len; ++i)
    str[i] = random_table_[u_(e_)];
}