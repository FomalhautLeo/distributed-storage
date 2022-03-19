#include <iostream>
#include <memory>

#include "CacheServer/CacheManager.h"
#include "CacheServer/CacheChange.h"

// The capacity of one CacheServer.
static const int kCapacity = 1000;

// Server number:
// Primary Standby
//   11      21
//   12      22
//   13      23
//   ...     ...

int main(int argc, char* argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: ./CacheServer {server_num}" << std::endl;
    exit(1);
  }

  auto temp_arg = argv[1];

  if (*(++temp_arg) > '3') {
    CacheChange::SetStatus({'e','l'});
    CacheChange::SetBusy(true);
  }
  // A pointer to manage CacheServer.
  auto cache_manager = std::make_unique<CacheManager>(argv[1], kCapacity);
  cache_manager->Run();
  cache_manager.reset();
  sleep(2);
  return 0;
}