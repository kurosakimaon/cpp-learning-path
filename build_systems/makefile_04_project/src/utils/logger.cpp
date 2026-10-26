#include "logger.h"

#include <iostream>

namespace demo {

void log(LogLevel level, const std::string& message) {
    const char* prefix = (level == LogLevel::Warn) ? "[警告] " : "[信息] ";
    std::cout << prefix << message << std::endl;
}

} // namespace demo
