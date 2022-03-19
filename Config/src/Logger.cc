#include <atomic>
#include <fstream>
#include <string.h>

#include "Config/Logger.h"

namespace ServerLog {
// 2: Debug, 3: Info, 4: Warn, 5: Error, 6:Fatal
int log_level  = 2;
int colorful  = 0;
bool show_time  = true;
std::ostream* log_stream = &std::cout;
std::ostream* ofs;
}  // namespace ServerLog

void ServerLog::SetLogPath(const char* log_path) {
  ofs = new std::ofstream(log_path, std::ios_base::out | std::ios_base::app);
}

int ServerLog::LogLevel() {
  return log_level;
}

using ServerLog::Logger;

Logger Logger::silencer;

static const char *kLoggerColors[]{
  "",
  0,
  "\033[36m",       // DEBUG: Cyan
  "\033[32m",       // INFO:  Green
  "\033[33m",       // WARN:  Yellow
  "\033[31m",       // ERROR: Red
  "\033[1m\033[31m" // FATAL: Bold Red
};

static const char *kLoggerFlags[]{
  "TEST",
  0,
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};

static std::atomic_flag kLoggerLockLog = ATOMIC_FLAG_INIT;

void Logger::Lock() {
  while(kLoggerLockLog.test_and_set());
}

void Logger::Unlock() {
  kLoggerLockLog.clear();
}

static std::string prefix;

Logger::Logger(int flag) : active_(true) {
  prefix.clear();
  if ((colored_ = (flag >= colorful)))
    prefix += kLoggerColors[flag];
  prefix += "[";
  prefix += kLoggerFlags[flag];
  prefix += "]";
  if (show_time) {
    prefix += "\t  ";
    time_t t = time(0);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    prefix += buf;
  }
  prefix += ":  ";
  while(kLoggerLockLog.test_and_set()) { }
  (*log_stream) << prefix;
  // Output to log file.
  (*ofs) << prefix;
}

Logger::~Logger()
{
  if (active_) {
    if (colored_) {
      (*log_stream) << "\033[0m";
    }
    kLoggerLockLog.clear();
  }
}
