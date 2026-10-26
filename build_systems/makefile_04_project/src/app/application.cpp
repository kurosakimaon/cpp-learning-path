#include "application.h"

#include "calculator.h"
#include "logger.h"
#include "string_helper.h"

#include <string>
#include <vector>

namespace demo {

int Application::run() const {
    Calculator calculator;

    log(LogLevel::Info, "项目构建成功，开始演示。");

    const int a = 6;
    const int b = 7;
    log(LogLevel::Info, std::to_string(a) + " + " + std::to_string(b) +
                            " = " + std::to_string(calculator.add(a, b)));
    log(LogLevel::Info, std::to_string(a) + " * " + std::to_string(b) +
                            " = " + std::to_string(calculator.multiply(a, b)));

    const std::vector<std::string> words{"makefile", "project", "demo"};
    log(LogLevel::Info, "拼接结果：" + join(words, " / "));
    log(LogLevel::Info, "转大写：" + toUpper("hello, makefile"));

#ifdef DEBUG
    log(LogLevel::Warn, "当前是 Debug 构建（定义了 DEBUG 宏）。");
#else
    log(LogLevel::Info, "当前是 Release 构建（未定义 DEBUG 宏）。");
#endif

    return 0;
}

} // namespace demo
