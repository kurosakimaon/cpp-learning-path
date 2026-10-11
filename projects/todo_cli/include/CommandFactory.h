// ================================================================
//  CommandFactory.h —— 命令工厂
//  根据用户输入创建对应的命令对象
// ================================================================
//
//  徒弟啊，这就是工厂模式（Factory Pattern）。
//
//  什么是工厂模式？
//  把"创建对象"的逻辑封装到一个专门的工厂类里，
//  调用者不用知道具体怎么创建，只要告诉工厂"我要什么"，
//  工厂就给你返回对应的对象。
//
//  在我们的项目里：
//  用户输入 "add 写作业" → 工厂解析 → 创建 AddCommand
//  用户输入 "delete 1"   → 工厂解析 → 创建 DeleteCommand
//
//  为什么要用工厂？
//    1. 把命令解析的逻辑集中在一个地方
//    2. 调用者（ConsoleUI）不用知道有哪些具体命令类
//    3. 新增命令只要在工厂里加一行，上层不用改
//
//  命令模式 + 工厂模式，简直是绝配！
//

#ifndef TODO_CLI_COMMANDFACTORY_H
#define TODO_CLI_COMMANDFACTORY_H

#include "Command.h"
#include <string>
#include <vector>
#include <memory>  // unique_ptr

namespace todo {

class CommandFactory {
public:
    // 解析用户输入，创建对应的命令对象
    // 返回 unique_ptr<Command>：
    //   - 用智能指针管理内存，不用手动 delete
    //   - 工厂创建，调用者使用，用完自动释放
    //
    // 参数：用户输入的一整行字符串
    // 返回：命令对象（解析失败返回 nullptr）
    //
    static std::unique_ptr<Command> parse(const std::string& input);

private:
    // 辅助：把一行输入拆分成"命令名 + 参数列表"
    static std::vector<std::string> splitArgs(const std::string& input);

    // 解析 add 命令的参数（-p, -d, -t 等选项）
    static std::unique_ptr<Command> parseAddCommand(const std::vector<std::string>& args);

    // 解析 list 命令的参数
    static std::unique_ptr<Command> parseListCommand(const std::vector<std::string>& args);
};

} // namespace todo

#endif // TODO_CLI_COMMANDFACTORY_H
