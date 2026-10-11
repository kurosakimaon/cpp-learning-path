// ================================================================
//  CommandFactory.cpp —— 命令工厂实现
// ================================================================

#include "CommandFactory.h"
#include <sstream>
#include <cctype>

namespace todo {

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
        } else if (c == ' ' && !in_quotes) {
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
        if (cmd_args.empty()) {
            return nullptr;
        }
        try {
            int id = std::stoi(cmd_args[0]);
            return std::make_unique<DeleteCommand>(id);
        } catch (...) {
            return nullptr;
        }
    }
    else if (cmd == "list" || cmd == "ls" || cmd == "l") {
        return parseListCommand(cmd_args);
    }
    else if (cmd == "complete" || cmd == "done" || cmd == "c") {
        if (cmd_args.empty()) {
            return nullptr;
        }
        try {
            int id = std::stoi(cmd_args[0]);
            return std::make_unique<CompleteCommand>(id);
        } catch (...) {
            return nullptr;
        }
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
        if (args[i] == "-p" && i + 1 < args.size()) {
            // 优先级
            std::string p = args[i + 1];
            for (char& c : p) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (p == "high" || p == "h") priority = Priority::High;
            else if (p == "low" || p == "l") priority = Priority::Low;
            else priority = Priority::Medium;
            i += 2;
        } else if (args[i] == "-d" && i + 1 < args.size()) {
            // 截止日期
            due_date = args[i + 1];
            i += 2;
        } else if (args[i] == "-t" && i + 1 < args.size()) {
            // 标签（逗号分隔）
            std::string tags_str = args[i + 1];
            std::string tag;
            std::istringstream iss(tags_str);
            while (std::getline(iss, tag, ',')) {
                if (!tag.empty()) tags.push_back(tag);
            }
            i += 2;
        } else {
            // 标题部分
            title_parts.push_back(args[i]);
            ++i;
        }
    }

    // 拼接标题
    for (size_t j = 0; j < title_parts.size(); ++j) {
        if (j > 0) title += " ";
        title += title_parts[j];
    }

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
        if (args[i] == "-sort" && i + 1 < args.size()) {
            cmd->setSortBy(args[i + 1]);
            i += 2;
        } else if (args[i] == "-tag" && i + 1 < args.size()) {
            cmd->setFilterTag(args[i + 1]);
            i += 2;
        } else if (args[i] == "-pending") {
            cmd->setShowPendingOnly(true);
            ++i;
        } else {
            ++i;  // 忽略未知参数
        }
    }

    return cmd;
}

} // namespace todo
