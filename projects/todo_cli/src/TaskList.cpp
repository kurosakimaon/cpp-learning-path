// ================================================================
//  TaskList.cpp —— 任务列表类实现
// ================================================================

#include "TaskList.h"
#include <algorithm>

namespace todo {

// ================================================================
//  构造函数
// ================================================================

TaskList::TaskList()
    : next_id_(1) {  // ID 从 1 开始，0 表示无效
}

// ================================================================
//  添加任务
// ================================================================

int TaskList::addTask(const Task& task) {
    Task new_task = task;
    new_task.setId(next_id_);   // 自动分配 ID
    tasks_.push_back(new_task); // 添加到列表
    return next_id_++;          // ID 自增，返回旧 ID
}

// ================================================================
//  删除任务
// ================================================================

bool TaskList::deleteTask(int id) {
    // 找到对应 ID 的任务
    auto it = std::find_if(tasks_.begin(), tasks_.end(),
        [id](const Task& t) { return t.getId() == id; });

    if (it == tasks_.end()) {
        return false;  // 没找到
    }

    tasks_.erase(it);  // 删除
    return true;
}

// ================================================================
//  查找任务
// ================================================================

Task* TaskList::findTask(int id) {
    for (auto& task : tasks_) {
        if (task.getId() == id) {
            return &task;
        }
    }
    return nullptr;
}

const Task* TaskList::findTask(int id) const {
    for (const auto& task : tasks_) {
        if (task.getId() == id) {
            return &task;
        }
    }
    return nullptr;
}

// ================================================================
//  完成状态切换
// ================================================================

bool TaskList::completeTask(int id) {
    Task* task = findTask(id);
    if (!task) return false;
    task->setCompleted(true);
    return true;
}

bool TaskList::uncompleteTask(int id) {
    Task* task = findTask(id);
    if (!task) return false;
    task->setCompleted(false);
    return true;
}

bool TaskList::toggleTask(int id) {
    Task* task = findTask(id);
    if (!task) return false;
    task->toggleCompleted();
    return true;
}

// ================================================================
//  排序（策略模式）
// ================================================================
//
//  策略模式的核心：
//  排序的逻辑（算法）是固定的（std::sort），
//  但"怎么比较"是可变的，通过参数传进来。
//
//  这样，以后要加新的排序方式，
//  只要写一个新的比较函数，不用改 sorted 方法本身。
//

std::vector<Task> TaskList::sorted(SortComparator comparator) const {
    // 复制一份，不修改原列表
    std::vector<Task> result = tasks_;
    std::sort(result.begin(), result.end(), comparator);
    return result;
}

// ---- 内置排序策略 ----

SortComparator TaskList::sortById() {
    return [](const Task& a, const Task& b) {
        return a.getId() < b.getId();
    };
}

SortComparator TaskList::sortByPriority() {
    // 优先级高的排前面
    // High(2) > Medium(1) > Low(0)
    return [](const Task& a, const Task& b) {
        // 先按优先级（高在前）
        int pa = static_cast<int>(a.getPriority());
        int pb = static_cast<int>(b.getPriority());
        if (pa != pb) return pa > pb;
        // 优先级相同按 ID
        return a.getId() < b.getId();
    };
}

SortComparator TaskList::sortByDueDate() {
    // 有截止日期的排前面，早的排前面
    return [](const Task& a, const Task& b) {
        const std::string& da = a.getDueDate();
        const std::string& db = b.getDueDate();

        // 都没有截止日期 → 按 ID
        if (da.empty() && db.empty()) return a.getId() < b.getId();
        // 一个有一个没有 → 有的排前面
        if (da.empty()) return false;
        if (db.empty()) return true;
        // 都有 → 按日期字符串比较（YYYY-MM-DD 格式可以直接比）
        if (da != db) return da < db;
        return a.getId() < b.getId();
    };
}

SortComparator TaskList::sortByCreatedDate() {
    // 新创建的排前面
    return [](const Task& a, const Task& b) {
        return a.getCreatedAt() > b.getCreatedAt();
    };
}

// ================================================================
//  筛选（策略模式）
// ================================================================

std::vector<Task> TaskList::filtered(FilterPredicate predicate) const {
    std::vector<Task> result;
    for (const auto& task : tasks_) {
        if (predicate(task)) {
            result.push_back(task);
        }
    }
    return result;
}

// ---- 内置筛选策略 ----

FilterPredicate TaskList::filterByCompleted(bool completed) {
    return [completed](const Task& t) {
        return t.isCompleted() == completed;
    };
}

FilterPredicate TaskList::filterByTag(const std::string& tag) {
    return [tag](const Task& t) {
        return t.hasTag(tag);
    };
}

FilterPredicate TaskList::filterByPriority(Priority p) {
    return [p](const Task& t) {
        return t.getPriority() == p;
    };
}

} // namespace todo
