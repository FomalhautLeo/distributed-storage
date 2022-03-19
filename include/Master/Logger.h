#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#pragma warning (disable:4996)

/*
a log class ,used to record system operation information
The file is unlocked,need to be improved
*/

class Logger{
public:
    enum log_level { debug, info, warning, error };//log level：from low to high 
    enum log_target { file, terminal, file_and_terminal };// log output destination
    enum log_colour { blue, yellow, green, red}; //change string colour
public:
    Logger();
    Logger(log_target target, log_level level, const std::string& path);
    ~Logger();
    
    void Debug(const std::string& text);
    void Info(const std::string& text);
    void Warning(const std::string& text);
    void Errors(const std::string& text);

private:
    std::ofstream m_outfile_;    
    log_target m_target_;        
    std::string m_path_;             
    log_level m_level_;    
    std::mutex file_mutex_;      
    void OutPut(const std::string &text, log_level act_level);          
};

#endif//_LOGGER_H_