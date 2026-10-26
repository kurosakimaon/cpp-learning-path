// ================================================================
//  Command.cpp —— 各命令实现
// ================================================================

#include "Command.h"
#include <cctype>
#include <iostream>
#include <sstream>

namespace todo {

namespace {

// 截止日期统一用 YYYY-MM-DD 格式。
// 因为排序直接按字符串比较，只有这个格式下字典序才等于时间顺序。
bool isValidDate(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') {
        return false;
    }
    for (size_t i : {0u, 1u, 2u, 3u, 5u, 6u, 8u, 9u}) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }

    const int year = std::stoi(s.substr(0, 4));
    const int month = std::stoi(s.substr(5, 2));
    const int day = std::stoi(s.substr(8, 2));

    if (month < 1 || month > 12) {
        return false;
    }
    static const int days_per_month[] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    const int max_day = days_per_month[month - 1] + (month == 2 && leap ? 1 : 0);

    return day >= 1 && day <= max_day;
}

} // namespace

// ================================================================
//  AddCommand —— 添加任务
// ================================================================

AddCommand::AddCommand(std::string title,
                       Priority priority,
                       std::string due_date,
                       std::vector<std::string> tags)
    : title_(std::move(title))
    , priority_(priority)
    , due_date_(std::move(due_date))
    , tags_(std::move(tags)) {
}

CommandResult AddCommand::execute(TaskList& tasks, Storage& storage) {
    if (!due_date_.empty() && !isValidDate(due_date_)) {
        return CommandResult::error(
            "截止日期格式不对，应为 YYYY-MM-DD（例如 2024-12-31）");
    }

    Task task(title_);
    task.setPriority(priority_);
    task.setDueDate(due_date_);
    for (const auto& tag : tags_) {
        task.addTag(tag);
    }

    int id = tasks.addTask(task);

    // 自动保存（可选，这里我们每次修改都自动保存）
    // 如果不想每次都存，可以改成手动 save 命令
    // 注意：保存失败要告诉用户，否则用户以为数据已经落盘了。
    if (!storage.save(tasks.getAllTasks())) {
        std::ostringstream oss;
        oss << "已添加任务 #" << id << ": " << title_
            << "，但保存到文件失败（数据只在内存中）";
        return CommandResult::error(oss.str());
    }

    std::ostringstream oss;
    oss << "已添加任务 #" << id << ": " << title_;
    return CommandResult::ok(oss.str());
}

// ================================================================
//  DeleteCommand —— 删除任务
// ================================================================

DeleteCommand::DeleteCommand(int task_id)
    : task_id_(task_id) {
}

CommandResult DeleteCommand::execute(TaskList& tasks, Storage& storage) {
    const Task* task = tasks.findTask(task_id_);
    if (!task) {
        std::ostringstream oss;
        oss << "找不到任务 #" << task_id_;
        return CommandResult::error(oss.str());
    }

    std::string title = task->getTitle();
    tasks.deleteTask(task_id_);

    if (!storage.save(tasks.getAllTasks())) {
        std::ostringstream oss;
        oss << "已删除任务 #" << task_id_ << ": " << title
            << "，但保存到文件失败（数据只在内存中）";
        return CommandResult::error(oss.str());
    }

    std::ostringstream oss;
    oss << "已删除任务 #" << task_id_ << ": " << title;
    return CommandResult::ok(oss.str());
}

// ================================================================
//  ListCommand —— 列出任务
// ================================================================

ListCommand::ListCommand()
    : sort_by_("id")
    , filter_tag_("")
    , pending_only_(false) {
}

CommandResult ListCommand::execute(TaskList& tasks, Storage&) {
    // 第一步：获取排序后的列表
    std::vector<Task> sorted;
    if (sort_by_ == "priority") {
        sorted = tasks.sorted(TaskList::sortByPriority());
    } else if (sort_by_ == "date") {
        sorted = tasks.sorted(TaskList::sortByDueDate());
    } else {
        // 默认按 ID
        sorted = tasks.sorted(TaskList::sortById());
    }

    // 第二步：筛选
    std::vector<Task> result;
    for (const auto& task : sorted) {
        // 按状态筛选
        if (pending_only_ && task.isCompleted()) {
            continue;
        }
        // 按标签筛选
        if (!filter_tag_.empty() && !task.hasTag(filter_tag_)) {
            continue;
        }
        result.push_back(task);
    }

    // 第三步：输出
    std::ostringstream oss;

    if (result.empty()) {
        oss << "（没有任务）" << std::endl;
    } else {
        int completed_count = 0;
        for (const auto& task : result) {
            oss << "  " << task.toString() << std::endl;
            if (task.isCompleted()) completed_count++;
        }
        oss << std::endl;
        oss << "共 " << result.size() << " 个任务"
            << "（已完成 " << completed_count
            << "，待完成 " << (result.size() - completed_count) << "）";
    }

    return CommandResult::ok(oss.str());
}

// ================================================================
//  CompleteCommand —— 标记完成
// ================================================================

CompleteCommand::CompleteCommand(int task_id)
    : task_id_(task_id) {
}

CommandResult CompleteCommand::execute(TaskList& tasks, Storage& storage) {
    const Task* task = tasks.findTask(task_id_);
    if (!task) {
        std::ostringstream oss;
        oss << "找不到任务 #" << task_id_;
        return CommandResult::error(oss.str());
    }

    // 切换状态（完成 ↔ 未完成）
    tasks.toggleTask(task_id_);

    // 重新获取（因为 toggle 后状态变了）
    task = tasks.findTask(task_id_);

    std::string state = task->isCompleted() ? "已完成" : "已取消完成";
    std::string title = task->getTitle();

    if (!storage.save(tasks.getAllTasks())) {
        std::ostringstream oss;
        oss << state << "任务 #" << task_id_ << ": " << title
            << "，但保存到文件失败（数据只在内存中）";
        return CommandResult::error(oss.str());
    }

    std::ostringstream oss;
    oss << state << "任务 #" << task_id_ << ": " << title;
    return CommandResult::ok(oss.str());
}

// ================================================================
//  SaveCommand —— 保存
// ================================================================

CommandResult SaveCommand::execute(TaskList& tasks, Storage& storage) {
    if (storage.save(tasks.getAllTasks())) {
        return CommandResult::ok("已保存到文件");
    } else {
        return CommandResult::error("保存失败");
    }
}

// ================================================================
//  LoadCommand —— 加载
// ================================================================

CommandResult LoadCommand::execute(TaskList& tasks, Storage& storage) {
    // 文件不存在时不能把内存里的任务清空：
    // 用户可能是误删了文件，直接清空再自动保存会把数据彻底弄丢。
    if (!storage.exists()) {
        return CommandResult::error(
            "文件不存在，当前任务未改变（可先用 save 创建文件）");
    }

    std::vector<Task> loaded;
    if (!storage.load(loaded)) {
        return CommandResult::error("加载失败，当前任务未改变");
    }

    // 加载成功后才清空旧任务并导入文件中的任务。
    // importTasks 会保留原 ID，并把 next_id_ 更新为“最大 ID + 1”，
    // 这样后续 add 不会和已加载任务的 ID 撞车。
    tasks.clear();
    tasks.importTasks(loaded);

    std::ostringstream oss;
    oss << "已加载 " << loaded.size() << " 个任务";
    return CommandResult::ok(oss.str());
}

// ================================================================
//  HelpCommand —— 帮助
// ================================================================

CommandResult HelpCommand::execute(TaskList&, Storage&) {
    std::ostringstream oss;
    oss << "可用命令：" << std::endl;
    oss << "  add <标题> [-p high|medium|low] [-d YYYY-MM-DD] [-t 标签1,标签2]" << std::endl;
    oss << "      添加新任务" << std::endl;
    oss << "  delete <ID>" << std::endl;
    oss << "      删除指定任务" << std::endl;
    oss << "  list [-sort id|priority|date] [-tag 标签] [-pending]" << std::endl;
    oss << "      列出所有任务（可排序、可筛选）" << std::endl;
    oss << "  complete <ID>" << std::endl;
    oss << "      标记任务完成/取消完成" << std::endl;
    oss << "  save" << std::endl;
    oss << "      手动保存到文件" << std::endl;
    oss << "  load" << std::endl;
    oss << "      从文件重新加载" << std::endl;
    oss << "  help" << std::endl;
    oss << "      显示帮助信息" << std::endl;
    oss << "  exit" << std::endl;
    oss << "      退出程序" << std::endl;
    oss << std::endl;
    oss << "示例：" << std::endl;
    oss << "  add 写作业 -p high -d 2024-12-31 -t 学习,重要" << std::endl;
    oss << "  list -sort priority -pending" << std::endl;
    oss << "  complete 1" << std::endl;

    return CommandResult::ok(oss.str());
}

// ================================================================
//  ExitCommand —— 退出
// ================================================================

CommandResult ExitCommand::execute(TaskList&, Storage&) {
    // 修改类命令（add/delete/complete）在每次操作后已经自动保存；
    // 退出时不再无条件重写文件。这样只读文件或未做修改时退出，
    // 不会出现"保存失败"的误报，也不会无谓地重排文件内容。
    // 如果之前的自动保存失败，命令当时已经明确提示过用户。
    return CommandResult::exit();
}

} // namespace todo
