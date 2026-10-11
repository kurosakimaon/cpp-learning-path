// ================================================================
//  Command.h —— 命令接口（命令模式）
//  每个操作都是一个命令对象
// ================================================================
//
//  徒弟啊，这就是命令模式（Command Pattern）。
//
//  什么是命令模式？
//  把"请求"封装成一个对象，这样你就可以：
//    - 用不同的请求参数化对象
//    - 把请求排队、记录日志
//    - 支持撤销操作
//
//  在我们这个项目里，命令模式的好处是：
//    1. 每个操作（添加、删除、列出...）都是独立的类
//       职责清晰，互不干扰
//    2. 新增操作只要加一个新的 Command 子类
//       不用修改已有代码（开闭原则）
//    3. 把"解析命令"和"执行命令"分开
//       代码结构更清晰
//
//  命令模式的核心就是这个 Command 接口：
//  只有一个 execute() 方法，执行命令。
//

#ifndef TODO_CLI_COMMAND_H
#define TODO_CLI_COMMAND_H

#include <string>
#include <vector>
#include "TaskList.h"
#include "Storage.h"

namespace todo {

// ================================================================
//  命令执行结果
// ================================================================
struct CommandResult {
    bool success;        // 是否成功
    std::string message; // 结果消息（显示给用户）
    bool should_exit;    // 是否应该退出程序（退出命令用）

    CommandResult()
        : success(true), should_exit(false) {}

    // 快速构造成功/失败结果
    static CommandResult ok(const std::string& msg = "") {
        CommandResult r;
        r.success = true;
        r.message = msg;
        return r;
    }

    static CommandResult error(const std::string& msg) {
        CommandResult r;
        r.success = false;
        r.message = msg;
        return r;
    }

    static CommandResult exit() {
        CommandResult r;
        r.success = true;
        r.should_exit = true;
        return r;
    }
};

// ================================================================
//  命令基类
// ================================================================
class Command {
public:
    virtual ~Command() = default;

    // 执行命令
    // 参数：任务列表、存储接口
    // 返回：执行结果
    virtual CommandResult execute(TaskList& tasks, Storage& storage) = 0;

    // 获取命令名称（用于显示）
    virtual std::string getName() const = 0;
};

// ================================================================
//  具体命令类
// ================================================================
//  下面是各种具体命令的声明
//  每个命令对应一个用户操作
//

// 添加任务命令
class AddCommand : public Command {
public:
    // 参数：标题、优先级、标签...
    explicit AddCommand(std::string title,
                        Priority priority = Priority::Medium,
                        std::string due_date = "",
                        std::vector<std::string> tags = {});

    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "add"; }

private:
    std::string title_;
    Priority priority_;
    std::string due_date_;
    std::vector<std::string> tags_;
};

// 删除任务命令
class DeleteCommand : public Command {
public:
    explicit DeleteCommand(int task_id);

    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "delete"; }

private:
    int task_id_;
};

// 列出任务命令
class ListCommand : public Command {
public:
    // 可选：排序方式、筛选条件
    ListCommand();

    // 设置排序方式
    void setSortBy(const std::string& sort_by) { sort_by_ = sort_by; }

    // 设置筛选标签
    void setFilterTag(const std::string& tag) { filter_tag_ = tag; }

    // 设置只显示未完成
    void setShowPendingOnly(bool v) { pending_only_ = v; }

    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "list"; }

private:
    std::string sort_by_;     // 排序方式：id/priority/date
    std::string filter_tag_;  // 筛选标签（空表示不过滤）
    bool pending_only_;       // 只显示未完成
};

// 标记完成命令
class CompleteCommand : public Command {
public:
    explicit CompleteCommand(int task_id);

    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "complete"; }

private:
    int task_id_;
};

// 保存命令
class SaveCommand : public Command {
public:
    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "save"; }
};

// 加载命令
class LoadCommand : public Command {
public:
    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "load"; }
};

// 帮助命令
class HelpCommand : public Command {
public:
    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "help"; }
};

// 退出命令
class ExitCommand : public Command {
public:
    CommandResult execute(TaskList& tasks, Storage& storage) override;
    std::string getName() const override { return "exit"; }
};

} // namespace todo

#endif // TODO_CLI_COMMAND_H
