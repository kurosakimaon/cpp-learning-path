/*
 * 示例4：应用程序类实现
 */
#include "application.h"
#include "logger.h"
#include "string_helper.h"
#include <iostream>

namespace app {

Application::Application() {
}

Application::~Application() {
}

bool Application::init() {
    LOG_INFO("应用程序初始化...");
    LOG_INFO("计算器模块就绪");
    LOG_INFO("日志模块就绪");
    return true;
}

int Application::run() {
    LOG_INFO("应用程序开始运行");

    demo_calculator();
    demo_string_utils();

    LOG_INFO("应用程序运行结束");
    LOG_INFO("总共进行了 " + std::to_string(calc_.get_count()) + " 次计算");
    return 0;
}

void Application::shutdown() {
    LOG_INFO("应用程序关闭中...");
}

void Application::demo_calculator() {
    std::cout << "\n===== 计算器演示 =====" << std::endl;

    double a = 10.0, b = 3.0;

    std::cout << a << " + " << b << " = " << calc_.add(a, b) << std::endl;
    std::cout << a << " - " << b << " = " << calc_.subtract(a, b) << std::endl;
    std::cout << a << " * " << b << " = " << calc_.multiply(a, b) << std::endl;
    std::cout << a << " / " << b << " = " << calc_.divide(a, b) << std::endl;
}

void Application::demo_string_utils() {
    std::cout << "\n===== 字符串工具演示 =====" << std::endl;

    std::string s = "  Hello, World!  ";
    std::cout << "原始字符串: '" << s << "'" << std::endl;
    std::cout << "trim 后: '" << utils::trim(s) << "'" << std::endl;
    std::cout << "转大写: " << utils::to_upper(utils::trim(s)) << std::endl;
    std::cout << "转小写: " << utils::to_lower(utils::trim(s)) << std::endl;

    std::string csv = "apple,banana,orange";
    auto parts = utils::split(csv, ',');
    std::cout << "\n分割 '" << csv << "':" << std::endl;
    for (size_t i = 0; i < parts.size(); i++) {
        std::cout << "  [" << i << "] " << parts[i] << std::endl;
    }

    std::cout << "用 ' - ' 重新拼接: " << utils::join(parts, " - ") << std::endl;
}

} // namespace app
