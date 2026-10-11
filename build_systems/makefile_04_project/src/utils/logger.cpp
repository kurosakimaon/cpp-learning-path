/*
 * 示例4：日志模块实现
 */
#include "logger.h"
#include <iostream>
#include <ctime>

namespace utils {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() : level_(LOG_DEBUG) {}

void Logger::set_level(LogLevel level) {
    level_ = level;
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < level_) return;  // 级别不够就不输出

    // 获取当前时间
    time_t now = time(nullptr);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::cout << "[" << time_buf << "] "
              << "[" << level_to_str(level) << "] "
              << msg << std::endl;
}

void Logger::debug(const std::string& msg) {
    log(LOG_DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(LOG_INFO, msg);
}

void Logger::warn(const std::string& msg) {
    log(LOG_WARN, msg);
}

void Logger::error(const std::string& msg) {
    log(LOG_ERROR, msg);
}

const char* Logger::level_to_str(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "?????";
    }
}

} // namespace utils
