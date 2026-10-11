// ================================================================
//  Task.cpp —— 任务类实现
// ================================================================

#include "Task.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace todo {

// ================================================================
//  构造函数
// ================================================================

Task::Task()
    : id_(0)
    , priority_(Priority::Medium)
    , completed_(false)
    , created_at_(time(nullptr)) {
}

Task::Task(const std::string& title)
    : id_(0)
    , title_(title)
    , priority_(Priority::Medium)
    , completed_(false)
    , created_at_(time(nullptr)) {
}

// ================================================================
//  优先级相关
// ================================================================

void Task::setPriorityFromString(const std::string& str) {
    // 转成小写再比较，这样用户输入 High/high/HIGH 都能识别
    std::string lower;
    for (char c : str) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (lower == "high" || lower == "h" || lower == "高") {
        priority_ = Priority::High;
    } else if (lower == "low" || lower == "l" || lower == "低") {
        priority_ = Priority::Low;
    } else {
        // 默认中优先级
        priority_ = Priority::Medium;
    }
}

std::string Task::getPriorityString() const {
    switch (priority_) {
        case Priority::High:   return "高";
        case Priority::Medium: return "中";
        case Priority::Low:    return "低";
        default:               return "中";
    }
}

// ================================================================
//  标签操作
// ================================================================

void Task::addTag(const std::string& tag) {
    // 先检查是否已经有这个标签，避免重复
    if (!hasTag(tag)) {
        tags_.push_back(tag);
    }
}

void Task::removeTag(const std::string& tag) {
    // 用 remove + erase 惯用法删除元素
    tags_.erase(
        std::remove(tags_.begin(), tags_.end(), tag),
        tags_.end()
    );
}

bool Task::hasTag(const std::string& tag) const {
    return std::find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

// ================================================================
//  辅助方法
// ================================================================

void Task::toggleCompleted() {
    completed_ = !completed_;
}

std::string Task::toString() const {
    // 构造一个人类可读的字符串
    std::ostringstream oss;

    // 完成状态
    oss << "[" << (completed_ ? "✓" : " ") << "] ";

    // ID
    oss << "#" << id_ << " ";

    // 标题
    oss << title_;

    // 优先级（高优先级标个星号）
    if (priority_ == Priority::High) {
        oss << " ⭐";
    } else if (priority_ == Priority::Low) {
        oss << " ·";
    }

    // 截止日期（如果有的话）
    if (!due_date_.empty()) {
        oss << " (截止: " << due_date_ << ")";
    }

    // 标签
    if (!tags_.empty()) {
        oss << " [";
        for (size_t i = 0; i < tags_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "#" << tags_[i];
        }
        oss << "]";
    }

    return oss.str();
}

// ================================================================
//  序列化与反序列化
// ================================================================
//
//  我们用简单的"竖线分隔"格式存文件，每行一个任务。
//  格式：id|title|description|priority|due_date|completed|created_at|tag1,tag2
//
//  为什么不用 JSON？
//  因为 JSON 需要第三方库（比如 nlohmann/json），
//  为了让项目不依赖外部库，我们用简单的文本格式。
//  等你以后学了第三方库，再改造成 JSON 也不难。
//

std::string Task::serialize() const {
    std::ostringstream oss;

    oss << id_ << "|";
    oss << title_ << "|";
    oss << description_ << "|";
    oss << getPriorityString() << "|";
    oss << due_date_ << "|";
    oss << (completed_ ? "1" : "0") << "|";
    oss << created_at_ << "|";

    // 标签用逗号分隔
    for (size_t i = 0; i < tags_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << tags_[i];
    }

    return oss.str();
}

// 辅助函数：按分隔符分割字符串
static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool Task::deserialize(const std::string& line, Task& out_task) {
    // 按 | 分割
    std::vector<std::string> parts = split(line, '|');

    // 至少需要 8 个字段（最后一个标签字段可以为空）
    if (parts.size() < 8) {
        return false;
    }

    try {
        out_task.setId(std::stoi(parts[0]));
        out_task.setTitle(parts[1]);
        out_task.setDescription(parts[2]);
        out_task.setPriorityFromString(parts[3]);
        out_task.setDueDate(parts[4]);
        out_task.setCompleted(parts[5] == "1");
        out_task.setCreatedAt(static_cast<time_t>(std::stoll(parts[6])));

        // 标签（用逗号分隔）
        if (!parts[7].empty()) {
            std::vector<std::string> tags = split(parts[7], ',');
            out_task.setTags(tags);
        }

        return true;
    } catch (...) {
        // 任何解析错误都返回 false
        return false;
    }
}

} // namespace todo
