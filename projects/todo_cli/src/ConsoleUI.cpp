// ================================================================
//  ConsoleUI.cpp —— 控制台用户界面实现
// ================================================================

#include "ConsoleUI.h"
#include "CommandFactory.h"
#include <iostream>
#include <memory>

namespace todo {

// ================================================================
//  构造函数
// ================================================================

ConsoleUI::ConsoleUI() = default;

// ================================================================
//  显示欢迎信息
// ================================================================

void ConsoleUI::showWelcome() {
    printSeparator();
    std::cout << "  欢迎使用 Todo CLI - 命令行待办事项工具" << std::endl;
    std::cout << "  输入 help 查看帮助，输入 exit 退出" << std::endl;
    printSeparator();
}

// ================================================================
//  主循环
// ================================================================

void ConsoleUI::run(TaskList& tasks, Storage& storage) {
    showWelcome();

    std::string input;
    bool running = true;

    while (running) {
        // 显示提示符
        std::cout << "> ";
        std::cout.flush();

        // 读取输入
        input = readInput();

        if (input.empty()) {
            continue;  // 空行忽略
        }

        // 用工厂解析命令
        std::unique_ptr<Command> cmd = CommandFactory::parse(input);

        if (!cmd) {
            showError("未知命令，输入 help 查看帮助");
            continue;
        }

        // 执行命令
        CommandResult result = cmd->execute(tasks, storage);

        // 显示结果
        if (!result.message.empty()) {
            if (result.success) {
                showResult(result.message);
            } else {
                showError(result.message);
            }
        }

        // 检查是否退出
        if (result.should_exit) {
            running = false;
        }
    }

    std::cout << "再见！" << std::endl;
}

// ================================================================
//  显示结果
// ================================================================

void ConsoleUI::showResult(const std::string& message, bool /*success*/) {
    std::cout << message << std::endl;
}

// ================================================================
//  显示错误
// ================================================================

void ConsoleUI::showError(const std::string& message) {
    std::cout << "❌ 错误: " << message << std::endl;
}

// ================================================================
//  读取输入
// ================================================================

std::string ConsoleUI::readInput() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// ================================================================
//  打印分隔线
// ================================================================

void ConsoleUI::printSeparator() {
    std::cout << "================================================================" << std::endl;
}

} // namespace todo
