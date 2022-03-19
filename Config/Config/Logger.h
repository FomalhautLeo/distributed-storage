#ifndef STORAGE_CONFIG_LOGGER_H_
#define STORAGE_CONFIG_LOGGER_H_

#include <iostream>
#include <string>

/*
a log class ,used to record system operation information
The file is unlocked,need to be improved
*/

namespace ServerLog {
// 2:Debug, 3: Info, 4:Warn, 5:Error, 6:Fatal
int LogLevel();
// Set the path of log file.
void SetLogPath(const char* log_path);
// The stream for log output, std::cout by default.
extern std::ostream* log_stream;
extern std::ostream* ofs;
class Logger {
 public:
  Logger() : active_(false) {}
  Logger(int flag);
  ~Logger();
  Logger& operator<<(std::ostream&(*_f)(std::ostream&)) {
    if (active_) {
      _f(*log_stream);
      _f(*ofs);
    }
    return *this;
  }
  template <typename T>
  Logger& operator<<(const T& t) {
    if (active_) {
      (*log_stream) << t;
      (*ofs) << t;
    }
    return *this;
  }
  static Logger silencer;
  static void Lock();
  static void Unlock();
 private:
  const bool active_;
  bool colored_;
};
}  // namespace ServerLog

using ServerLog::LogLevel;

#define SERVERLOG(Flag) ServerLog::Logger(\
        Flag\
        )

#define LogTest   (LogLevel()>0 ? ServerLog::Logger::silencer : SERVERLOG(0))
#define LogDebug  (LogLevel()>2 ? ServerLog::Logger::silencer : SERVERLOG(2))
#define LogInfo   (LogLevel()>3 ? ServerLog::Logger::silencer : SERVERLOG(3))
#define LogWarn   (LogLevel()>4 ? ServerLog::Logger::silencer : SERVERLOG(4))
#define LogError  (LogLevel()>5 ? ServerLog::Logger::silencer : SERVERLOG(5))
#define LogFatal  (LogLevel()>6 ? ServerLog::Logger::silencer : SERVERLOG(6))

#endif  // STORAGE_CONFIG_LOGGER_H_