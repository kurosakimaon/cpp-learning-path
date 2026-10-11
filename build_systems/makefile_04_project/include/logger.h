/*
 * 示例4：日志模块头文件
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace utils {

// 日志级别
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO  = 1,
    LOG_WARN  = 2,
    LOG_ERROR = 3
};

class Logger {
public:
    // 获取单例（简单实现）
    static Logger& instance();

    // 设置日志级别
    void set_level(LogLevel level);

    // 输出日志
    void log(LogLevel level, const std::string& msg);

    // 便捷方法
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger();
    LogLevel level_;

    // 级别转字符串
    const char* level_to_str(LogLevel level);
};

// 全局便捷宏
#define LOG_DEBUG(msg) utils::Logger::instance().debug(msg)
#define LOG_INFO(msg)  utils::Logger::instance().info(msg)
#define LOG_WARN(msg)  utils::Logger::instance().warn(msg)
#define LOG_ERROR(msg) utils::Logger::instance().error(msg)

} // namespace utils

#endif // LOGGER_H
