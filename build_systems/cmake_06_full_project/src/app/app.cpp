// ============================================================
//  app.cpp - 主应用类实现
// ============================================================

#include "app.h"
#include "myproject/version.h"
#include "mathutils/calculator.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

namespace myproject {

App::App()
    : welcome_message_("欢迎使用 MyProject 示例程序！")
{
}

App::~App() = default;

void App::setWelcomeMessage(const std::string& msg) {
    welcome_message_ = msg;
}

void App::printHelp() {
    std::cout << "用法：myproject_app [选项]" << std::endl;
    std::cout << "选项：" << std::endl;
    std::cout << "  -h, --help      显示帮助信息" << std::endl;
    std::cout << "  -v, --version   显示版本信息" << std::endl;
    std::cout << "  -d, --demo      运行演示（不加选项时只显示本帮助）" << std::endl;
}

void App::printVersion() {
    std::cout << "MyProject 版本 " << versionString() << std::endl;
    std::cout << "构建类型：" << BUILD_TYPE << std::endl;
}

int App::parseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;  // 打印完帮助就退出
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "-d" || arg == "--demo") {
            demo_requested_ = true;
        } else {
            std::cerr << "未知选项：" << arg << std::endl;
            printHelp();
            return 1;
        }
    }
    return -1;  // 参数都没问题，继续运行
}

int App::run(int argc, char* argv[]) {
    // 先解析参数
    int parse_result = parseArgs(argc, argv);
    if (parse_result >= 0) {
        return parse_result;
    }

    // 没要求运行演示，就只打印帮助
    if (!demo_requested_) {
        printHelp();
        return 0;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "  " << welcome_message_ << std::endl;
    std::cout << "  版本：" << versionString() << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // ---- 演示数学库的功能 ----
    std::cout << "【数学计算演示】" << std::endl;

    mathutils::Calculator calc;

    double a = 15.0;
    double b = 4.0;

    std::cout << "  " << a << " + " << b << " = " << calc.add(a, b) << std::endl;
    std::cout << "  " << a << " - " << b << " = " << calc.subtract(a, b) << std::endl;
    std::cout << "  " << a << " * " << b << " = " << calc.multiply(a, b) << std::endl;
    std::cout << "  " << a << " / " << b << " = " << calc.divide(a, b) << std::endl;
    std::cout << std::endl;

    // ---- 演示多线程（证明 Threads 库链接上了） ----
    std::cout << "【多线程演示】" << std::endl;
    std::cout << "  主线程 ID：" << std::this_thread::get_id() << std::endl;

    int result = 0;
    std::thread worker([&result]() {
        // 在子线程里做个计算
        mathutils::Calculator c;
        result = static_cast<int>(c.add(100, 200));
        std::cout << "  工作线程 ID：" << std::this_thread::get_id() << std::endl;
        std::cout << "  工作线程计算结果：100 + 200 = " << result << std::endl;
    });

    worker.join();
    std::cout << "  子线程已结束，结果：" << result << std::endl;
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "  程序正常结束。" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

} // namespace myproject
