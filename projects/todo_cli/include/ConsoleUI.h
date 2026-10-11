// ================================================================
//  ConsoleUI.h —— 控制台用户界面
//  负责和用户交互：显示菜单、读取输入、展示结果
// ================================================================
//
//  徒弟啊，这个类负责用户界面（UI）。
//  为什么要单独搞一个 UI 类？
//  因为我们要把"业务逻辑"和"界面展示"分开。
//
//  TaskList 是业务逻辑（管理任务），
//  ConsoleUI 是界面（和用户交互），
//  两者通过 Command 来沟通。
//
//  这样分层的好处：
//    - 以后想改成图形界面（Qt、Web...），业务逻辑不用改
//    - 界面改了，不影响核心逻辑
//    - 职责清晰，代码好维护
//
//  这就是"MVC"思想的简化版：
//  Model（Task/TaskList）- View（ConsoleUI）- Controller（Command）
//

#ifndef TODO_CLI_CONSOLEUI_H
#define TODO_CLI_CONSOLEUI_H

#include <string>
#include "TaskList.h"
#include "Storage.h"

namespace todo {

class ConsoleUI {
public:
    ConsoleUI();

    // 运行主循环
    // 参数：任务列表、存储接口
    void run(TaskList& tasks, Storage& storage);

    // 显示欢迎信息
    void showWelcome();

    // 显示命令结果
    void showResult(const std::string& message, bool success = true);

    // 显示错误信息
    void showError(const std::string& message);

    // 读取用户输入的一行
    std::string readInput();

private:
    // 打印分隔线
    void printSeparator();
};

} // namespace todo

#endif // TODO_CLI_CONSOLEUI_H
