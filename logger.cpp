#include <iostream>
#include <sstream>
#include <map>
#include <mutex>
#include "thread_pool.h"

#define BEGINS(x) namespace x {
#define ENDS(x) }

#define LOG(level) log_print::Logger::LoggerStream(level, __FILE__, __LINE__, log_print::my_log)
#define LOG_INFO LOG(log_print::LogLevel::INFO)
#define LOG_WARNNING LOG(log_print::LogLevel::WARNNING)
#define LOG_DEBUG LOG(log_print::LogLevel::DEBUG)
#define LOG_ERROR LOG(log_print::LogLevel::ERROR)
#define LOG_FATAL LOG(log_print::LogLevel::FATAL)
#define SET_LEVEL(level) log_print::my_log.set_level(level)

BEGINS(log_print)

class LogLevel {
public :
    static const int INFO;
    static const int WARNNING;
    static const int DEBUG;
    static const int ERROR; 
    static const int FATAL;
};

const int LogLevel::INFO = 1;
const int LogLevel::WARNNING = 2; 
const int LogLevel::DEBUG = 3;
const int LogLevel::ERROR = 4; 
const int LogLevel::FATAL = 5;

static std::map<int, std::string> LevelString = {
    {LogLevel::INFO, "INFO"},
    {LogLevel::WARNNING, "WARNNING"},
    {LogLevel::DEBUG, "DEBUG"},
    {LogLevel::ERROR, "ERROR"},
    {LogLevel::FATAL, "FATAL"}
};

class Logger {
public :
    Logger() : LOG_LEVEL(LogLevel::INFO) {}
    class LoggerStream : public std::ostringstream {
    public :
        LoggerStream(
            int level, const char *file_name, 
            int line_no, Logger &raw_log
            ) : line_no(line_no), level(level), raw_log(raw_log) {
            std::ostringstream &now = *this;
            now << "[" << file_name << " : " << LevelString[level] << "]";
        }
        ~LoggerStream() {
            if (level < raw_log.LOG_LEVEL) return ;
            std::unique_lock<std::mutex> lock(raw_log.m_mutex);
            std::cout << this->str() << " (" << line_no << ") " << std::endl;
        }
    private :
        int line_no, level;
        Logger &raw_log;
    };
    void set_level(int level) { this->LOG_LEVEL = level; }
    int LOG_LEVEL;
    std::mutex m_mutex;
};

Logger my_log;

ENDS(log_print)

void func(int a, int b, int c) {
    LOG_INFO << a << " " << b << " " << c;
    return ;
}

int main() {
    LOG_INFO << " hello world " << 123 << " " << 12.45;
    LOG_WARNNING << " hello world " << 123 << " " << 12.45;
    LOG_DEBUG <<  "hello world " << 123 << " " << 12.45;
    LOG_ERROR << " hello world " << 123 << " " << 12.45;
    LOG_FATAL << " hello world " << 123 << " " << 12.45;
    thread_pool::ThreadPool tp;
    for (int i = 0; i < 100; i++) {
        tp.addOneTask(func, i, 2 * i, 3 * i);
    }
    tp.stop();
    return 0;
}
