#include "Config/NetworkInfo.h"

std::shared_ptr<NetworkInfo> NetworkInfo::instance_ = nullptr;

std::shared_ptr<NetworkInfo> NetworkInfo::GetInstance() {
  if (instance_ == nullptr)
    instance_ = std::shared_ptr<NetworkInfo>(new NetworkInfo());
  return instance_;
}