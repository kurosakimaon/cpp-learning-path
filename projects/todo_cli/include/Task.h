// ================================================================
//  Task.h —— 任务类（数据模型）
//  描述一个待办任务的所有属性
// ================================================================
//
//  徒弟啊，这是整个项目最基础的类——Task（任务）。
//  数据模型是软件的基石，所有上层功能都是围绕数据展开的。
//
//  一个任务包含哪些信息呢？
//    - ID：唯一标识，用来找到特定任务
//    - 标题：任务内容的简短描述
//    - 描述：详细说明（可选）
//    - 优先级：高/中/低
//    - 截止日期：什么时候要完成
//    - 标签：用来分类和筛选
//    - 是否完成：任务状态
//    - 创建时间：什么时候加进来的
//
//  设计数据模型的时候，要想清楚：
//  哪些字段是必须的，哪些是可选的，
//  用什么类型最合适，怎么存最方便。
//

#ifndef TODO_CLI_TASK_H
#define TODO_CLI_TASK_H

#include <string>
#include <vector>
#include <ctime>

namespace todo {

// ================================================================
//  优先级枚举
// ================================================================
//  用 enum class 而不是普通 enum，更安全（强类型）。
enum class Priority {
    Low,     // 低优先级
    Medium,  // 中优先级（默认）
    High     // 高优先级
};

// ================================================================
//  任务类
// ================================================================
//  这是一个典型的"数据类"（Data Class），
//  主要用来保存数据，附带一些简单的操作方法。
//
//  注意：我们把成员变量设为私有，通过 getter/setter 访问，
//  这是封装的基本做法。
//
class Task {
public:
    // ---- 构造函数 ----
    Task();

    // 带标题的构造函数（最常用）
    explicit Task(const std::string& title);

    // ---- 基本属性的 getter/setter ----

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) { title_ = title; }

    const std::string& getDescription() const { return description_; }
    void setDescription(const std::string& desc) { description_ = desc; }

    Priority getPriority() const { return priority_; }
    void setPriority(Priority p) { priority_ = p; }

    // 用字符串设置优先级（方便用户输入）
    void setPriorityFromString(const std::string& str);

    // 把优先级转成字符串（方便显示）
    std::string getPriorityString() const;

    const std::string& getDueDate() const { return due_date_; }
    void setDueDate(const std::string& date) { due_date_ = date; }

    bool isCompleted() const { return completed_; }
    void setCompleted(bool v) { completed_ = v; }

    time_t getCreatedAt() const { return created_at_; }
    void setCreatedAt(time_t t) { created_at_ = t; }

    // ---- 标签操作 ----
    // 标签用 vector<string> 存，一个任务可以有多个标签

    const std::vector<std::string>& getTags() const { return tags_; }
    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }

    // ---- 辅助方法 ----

    // 标记完成/未完成
    void toggleCompleted();

    // 转成可读的字符串（用于显示）
    std::string toString() const;

    // 转成序列化字符串（用于保存到文件）
    // 格式：id|title|description|priority|due_date|completed|created_at|tag1,tag2
    // 字段内的 \、|、, 会用反斜杠转义
    std::string serialize() const;

    // 从序列化字符串解析
    // 返回是否解析成功
    static bool deserialize(const std::string& line, Task& out_task);

private:
    int id_;                    // 任务 ID（唯一标识）
    std::string title_;         // 标题
    std::string description_;   // 详细描述
    Priority priority_;         // 优先级
    std::string due_date_;      // 截止日期（字符串格式，简化处理）
    bool completed_;            // 是否完成
    time_t created_at_;         // 创建时间戳
    std::vector<std::string> tags_;  // 标签列表
};

} // namespace todo

#endif // TODO_CLI_TASK_H
