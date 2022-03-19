#include <memory>
#include <string>

#include "Client/ClientManager.h"

int main(int argc, char* argv[]) {
  std::string speed = argc < 2 ? "1" : argv[1];
  auto client_manager = std::make_unique<ClientManager>(std::stoi(speed));
  client_manager->Run();
  return 0;
}