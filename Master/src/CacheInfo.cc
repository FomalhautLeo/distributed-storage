#include "Master/CacheInfo.h"

// Initialization of static members
unsigned short CacheInfo::alive_cache_number_(0);
unsigned short CacheInfo::dead_cache_number_(0);
unsigned short CacheInfo::latest_cache_index_(0);



CacheInfo::CacheInfo(bool status) : cache_status_(status) {}
CacheInfo::CacheInfo(bool status,std::string ip,int port,unsigned short count) : 
    cache_status_(status),cache_ip_(ip),cache_port_(port),cache_count_(count){}
CacheInfo::CacheInfo(const CacheInfo& param){
  this->cache_status_=param.cache_status_;
  this->cache_ip_=param.cache_status_;
  this->cache_port_=param.cache_port_;
  this->cache_count_=param.cache_count_;
}

unsigned short CacheInfo::GetAliveCacheNumber() {
  return alive_cache_number_;
}

void CacheInfo::SetAliveCacheNumber(unsigned short param) {
  alive_cache_number_ = param;
}

unsigned short CacheInfo::GetDeadCacheNumber() {
  return dead_cache_number_;
}

void CacheInfo::SetDeadCacheNumber(unsigned short param) {
  dead_cache_number_ = param;
}

unsigned short CacheInfo::GetLatestCacheIndex() {
  return latest_cache_index_;
}

void CacheInfo::SetLatestCacheIndex(unsigned short param) {
  latest_cache_index_ = param;
}


std::string CacheInfo::GetCacheIp() const{
  return cache_ip_;
}

void CacheInfo::SetCacheIp(std::string param){
  cache_ip_=param;
}

int CacheInfo::GetCachePort() const{
  return cache_port_;
}

void CacheInfo::SetCachePort(int param){
  cache_port_=param;
}

unsigned short CacheInfo::GetCacheCount() const {
  return cache_count_;
}

void CacheInfo::SetCacheCount(unsigned short param){
  cache_count_=param;
  return;
}

bool CacheInfo::GetCacheStatus() const {
  return cache_status_;
}

void CacheInfo::SetCacheStatus(bool param) {
  cache_status_ = param;
}