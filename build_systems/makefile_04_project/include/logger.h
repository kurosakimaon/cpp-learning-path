#ifndef DEMO_LOGGER_H
#define DEMO_LOGGER_H

#include <string>

namespace demo {

enum class LogLevel {
    Info,
    Warn
};

void log(LogLevel level, const std::string& message);

} // namespace demo

#endif // DEMO_LOGGER_H
