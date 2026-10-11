// ================================================================
//  TaskList.h —— 任务列表类（核心业务逻辑）
//  管理所有任务，提供增删改查、排序、筛选等功能
// ================================================================
//
//  徒弟啊，Task 是单个任务，TaskList 就是任务的"大管家"。
//  它负责管理所有任务，提供各种操作：
//  - 添加任务
//  - 删除任务
//  - 查找任务
//  - 标记完成
//  - 排序（按优先级、按日期...）
//  - 筛选（按标签、按状态...）
//
//  为什么不直接用 vector<Task> 呢？
//  因为直接暴露底层容器，会让上层代码和具体实现耦合。
//  用 TaskList 封装一层，有这些好处：
//    1. 接口更清晰，调用者不用关心底层用什么容器
//    2. 可以在内部维护一些状态（比如下一个 ID）
//    3. 以后换底层实现（比如换成 list），上层不用改
//    4. 方便加业务逻辑（比如自动生成 ID、校验数据）
//

#ifndef TODO_CLI_TASKLIST_H
#define TODO_CLI_TASKLIST_H

#include "Task.h"
#include <vector>
#include <memory>   // unique_ptr
#include <functional> // function（用于策略模式）

namespace todo {

// ================================================================
//  比较函数类型（用于策略模式 - 排序）
// ================================================================
//  策略模式：把"怎么排序"封装成一个可调用对象，
//  调用时传不同的策略，就能得到不同的排序结果。
//
//  这里用 std::function 定义排序比较函数的类型：
//  接收两个 Task，返回 bool（第一个是否应该排在第二个前面）
//
using SortComparator = std::function<bool(const Task&, const Task&)>;

// ================================================================
//  筛选函数类型（用于策略模式 - 筛选）
// ================================================================
//  同理，筛选也用策略模式：
//  接收一个 Task，返回 bool（是否符合筛选条件）
//
using FilterPredicate = std::function<bool(const Task&)>;

// ================================================================
//  任务列表类
// ================================================================
//
//  这是整个应用的核心，所有业务逻辑都在这里。
//  上层（UI、命令）通过它来操作任务。
//
class TaskList {
public:
    TaskList();

    // ---- 增删改 ----

    // 添加任务，返回新任务的 ID
    int addTask(const Task& task);

    // 根据 ID 删除任务，返回是否成功（ID 不存在则失败）
    bool deleteTask(int id);

    // 根据 ID 查找任务（返回指针，找不到返回 nullptr）
    // 为什么返回指针而不是引用？
    // 因为可能找不到，引用不能是空的，指针可以。
    Task* findTask(int id);
    const Task* findTask(int id) const;

    // 标记任务完成/未完成
    bool completeTask(int id);
    bool uncompleteTask(int id);
    bool toggleTask(int id);

    // ---- 查询 ----

    // 获取所有任务数量
    size_t size() const { return tasks_.size(); }

    // 判断是否为空
    bool empty() const { return tasks_.empty(); }

    // 获取下一个可用的 ID
    int getNextId() const { return next_id_; }

    // 获取所有任务（只读）
    const std::vector<Task>& getAllTasks() const { return tasks_; }

    // ---- 排序（策略模式） ----
    //  返回排序后的任务列表（不修改原列表）
    //
    //  用法：
    //    auto sorted = list.sorted(sortByPriority);
    //    auto sorted = list.sorted(sortByDueDate);
    //
    std::vector<Task> sorted(SortComparator comparator) const;

    // 内置的几种排序策略
    static SortComparator sortById();          // 按 ID（默认）
    static SortComparator sortByPriority();    // 按优先级（高在前）
    static SortComparator sortByDueDate();     // 按截止日期（早在前）
    static SortComparator sortByCreatedDate(); // 按创建时间（新在前）

    // ---- 筛选（策略模式） ----
    //  返回符合条件的任务列表
    //
    std::vector<Task> filtered(FilterPredicate predicate) const;

    // 内置的几种筛选策略
    static FilterPredicate filterByCompleted(bool completed);   // 按完成状态
    static FilterPredicate filterByTag(const std::string& tag); // 按标签
    static FilterPredicate filterByPriority(Priority p);        // 按优先级

private:
    std::vector<Task> tasks_;  // 任务列表（底层用 vector 存）
    int next_id_;              // 下一个任务的 ID（自增）
};

} // namespace todo

#endif // TODO_CLI_TASKLIST_H
