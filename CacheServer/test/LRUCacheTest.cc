#include <iostream>
#include <string>

#include "CacheServer/LRUCache.h"

static LRUCache* lru;

void Put(const std::string& key, const std::string& value);
void Get(const std::string& key);

int main() {
  lru = new LRUCache(5);
  Put("a", "A");   // {a, A}
  Put("b", "B");   // {b, B}, {a, A}
  Put("c", "C");   // {c, C}, {b, B}, {a, A}
  Get("d");        // {c, C}, {b, B}, {a, A}
  Put("d", "D");   // {d, D}, {c, C}, {b, B}, {a, A}
  Get("b");        // {b, B}, {d, D}, {c, C}, {a, A}
  Put("a", "AA");  // {a, AA}, {b, B}, {d, D}, {c, C}
  Put("e", "E");   // {e, E}, {a, AA}, {b, B}, {d, D}, {c, C}
  Put("f", "F");   // {f, F}, {e, E}, {a, AA}, {b, B}, {d, D}
  delete lru;
  return 0;
}

void Put(const std::string& key, const std::string& value) {
  std::cout << "Put: " << key << ", " << value << std::endl;
  lru->Store(key, value);
  lru->PrintData();
}

void Get(const std::string& key) {
  std::cout << "Get: " << key << ", " << lru->Get(key) << std::endl;
  lru->PrintData();
}