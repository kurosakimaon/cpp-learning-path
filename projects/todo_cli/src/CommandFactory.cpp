// ================================================================
//  CommandFactory.cpp —— 命令工厂实现
// ================================================================

#include "CommandFactory.h"
#include "Utils.h"
#include <sstream>
#include <cctype>
#include <charconv>
#include <system_error>

namespace todo {

namespace {

// 解析任务 ID：必须整段都是数字、成功转换，且大于 0。
// 不能用 std::stoi 直接转：它只读前缀，"1.5" 会悄悄变成 1，"3xyz" 会变成 3。
bool parseTaskId(const std::string& text, int& id) {
    if (text.empty()) {
        return false;
    }
    const char* first = text.data();
    const char* last = first + text.size();
    auto [ptr, ec] = std::from_chars(first, last, id);
    return ec == std::errc{} && ptr == last && id > 0;
}

} // namespace

// ================================================================
//  辅助：分割字符串
// ================================================================

std::vector<std::string> CommandFactory::splitArgs(const std::string& input) {
    std::vector<std::string> args;
    std::string current;
    bool in_quotes = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
            // 空格、Tab 等都算分隔符
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        args.push_back(current);
    }

    return args;
}

// ================================================================
//  主解析函数
// ================================================================

std::unique_ptr<Command> CommandFactory::parse(const std::string& input) {
    std::vector<std::string> args = splitArgs(input);

    if (args.empty()) {
        return nullptr;  // 空输入
    }

    // 第一个参数是命令名，转成小写
    std::string cmd = args[0];
    for (char& c : cmd) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // 去掉命令名，剩下的是参数
    std::vector<std::string> cmd_args(args.begin() + 1, args.end());

    // 根据命令名创建对应的命令对象
    if (cmd == "add" || cmd == "a") {
        return parseAddCommand(cmd_args);
    }
    else if (cmd == "delete" || cmd == "del" || cmd == "d") {
        int id = 0;
        if (cmd_args.empty() || !parseTaskId(cmd_args[0], id)) {
            return nullptr;
        }
        return std::make_unique<DeleteCommand>(id);
    }
    else if (cmd == "list" || cmd == "ls" || cmd == "l") {
        return parseListCommand(cmd_args);
    }
    else if (cmd == "complete" || cmd == "done" || cmd == "c") {
        int id = 0;
        if (cmd_args.empty() || !parseTaskId(cmd_args[0], id)) {
            return nullptr;
        }
        return std::make_unique<CompleteCommand>(id);
    }
    else if (cmd == "save" || cmd == "s") {
        return std::make_unique<SaveCommand>();
    }
    else if (cmd == "load") {
        return std::make_unique<LoadCommand>();
    }
    else if (cmd == "help" || cmd == "h" || cmd == "?") {
        return std::make_unique<HelpCommand>();
    }
    else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        return std::make_unique<ExitCommand>();
    }

    // 未知命令
    return nullptr;
}

// ================================================================
//  解析 add 命令
// ================================================================
//  格式：add <标题> [-p high|medium|low] [-d 日期] [-t 标签1,标签2]
//

std::unique_ptr<Command> CommandFactory::parseAddCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return nullptr;  // 至少要有标题
    }

    std::string title;
    Priority priority = Priority::Medium;
    std::string due_date;
    std::vector<std::string> tags;

    // 第一个参数是标题（支持多词标题？简化处理：第一个参数是标题）
    // 更完善的做法是把所有非选项参数拼接成标题
    // 这里为了简单，第一个参数是标题
    // 或者我们换一种方式：收集所有"非选项"的参数作为标题

    std::vector<std::string> title_parts;
    size_t i = 0;

    while (i < args.size()) {
        if (args[i] == "-p") {
            if (i + 1 >= args.size()) {
                return nullptr;  // -p 后面缺少值
            }
            // 优先级
            std::string p = args[i + 1];
            for (char& c : p) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (p == "high" || p == "h") priority = Priority::High;
            else if (p == "medium" || p == "m") priority = Priority::Medium;
            else if (p == "low" || p == "l") priority = Priority::Low;
            else return nullptr;  // 不认识的优先级不能悄悄当成中优先级
            i += 2;
        } else if (args[i] == "-d") {
            if (i + 1 >= args.size()) {
                return nullptr;
            }
            // 截止日期
            due_date = args[i + 1];
            i += 2;
        } else if (args[i] == "-t") {
            if (i + 1 >= args.size()) {
                return nullptr;
            }
            // 标签（逗号分隔）
            std::string tags_str = args[i + 1];
            std::string tag;
            std::istringstream iss(tags_str);
            while (std::getline(iss, tag, ',')) {
                tag = utils::trim(tag);
                if (!tag.empty()) tags.push_back(tag);
            }
            i += 2;
        } else {
            // 标题部分
            title_parts.push_back(args[i]);
            ++i;
        }
    }

    // 拼接标题，并去掉首尾空白；全是空白的标题没有意义
    for (size_t j = 0; j < title_parts.size(); ++j) {
        if (j > 0) title += " ";
        title += title_parts[j];
    }
    title = utils::trim(title);

    if (title.empty()) {
        return nullptr;
    }

    return std::make_unique<AddCommand>(title, priority, due_date, tags);
}

// ================================================================
//  解析 list 命令
// ================================================================
//  格式：list [-sort id|priority|date] [-tag 标签] [-pending]
//

std::unique_ptr<Command> CommandFactory::parseListCommand(const std::vector<std::string>& args) {
    auto cmd = std::make_unique<ListCommand>();

    size_t i = 0;
    while (i < args.size()) {
        if (args[i] == "-sort") {
            if (i + 1 >= args.size()) {
                return nullptr;  // -sort 后面缺少值
            }
            const std::string& sort_by = args[i + 1];
            if (sort_by != "id" && sort_by != "priority" && sort_by != "date") {
                return nullptr;  // 拼错的排序方式不能悄悄退回按 ID
            }
            cmd->setSortBy(sort_by);
            i += 2;
        } else if (args[i] == "-tag") {
            if (i + 1 >= args.size()) {
                return nullptr;  // -tag 后面缺少值
            }
            cmd->setFilterTag(args[i + 1]);
            i += 2;
        } else if (args[i] == "-pending") {
            cmd->setShowPendingOnly(true);
            ++i;
        } else {
            return nullptr;  // 未知选项
        }
    }

    return cmd;
}

} // namespace todo
