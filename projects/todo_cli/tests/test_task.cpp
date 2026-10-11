// ================================================================
//  test_task.cpp —— 简单的单元测试
//  用 assert 做基础测试，不依赖第三方测试框架
// ================================================================
//
//  徒弟啊，单元测试很重要。
//  写完代码跑一遍测试，确保没改坏东西。
//
//  这里我们不用 gtest、Catch2 这些框架，
//  就用最朴素的 assert，简单直接。
//  等你以后学了测试框架，再升级也不迟。
//
//  测试的基本原则：
//    - 每个测试只测一件事
//    - 测试要独立，互不依赖
//    - 正常情况、边界情况、异常情况都要测
//

#include <iostream>
#include <cassert>
#include <string>

#include "Task.h"
#include "TaskList.h"

using namespace todo;

// ================================================================
//  测试 Task 类
// ================================================================

void test_task_basic() {
    std::cout << "  [测试] Task 基本功能..." << std::endl;

    Task task("学习 C++");

    // 基本属性
    assert(task.getTitle() == "学习 C++");
    assert(task.getPriority() == Priority::Medium);
    assert(!task.isCompleted());
    assert(task.getDescription().empty());

    std::cout << "    ✓ 通过" << std::endl;
}

void test_task_priority() {
    std::cout << "  [测试] Task 优先级..." << std::endl;

    Task task("test");

    // 默认中优先级
    assert(task.getPriorityString() == "中");

    // 设置为高优先级
    task.setPriorityFromString("high");
    assert(task.getPriority() == Priority::High);
    assert(task.getPriorityString() == "高");

    // 设置为低优先级
    task.setPriorityFromString("low");
    assert(task.getPriority() == Priority::Low);
    assert(task.getPriorityString() == "低");

    // 无效值 → 默认中
    task.setPriorityFromString("xxx");
    assert(task.getPriority() == Priority::Medium);

    std::cout << "    ✓ 通过" << std::endl;
}

void test_task_tags() {
    std::cout << "  [测试] Task 标签..." << std::endl;

    Task task("test");

    // 初始没有标签
    assert(task.getTags().empty());

    // 添加标签
    task.addTag("学习");
    task.addTag("重要");
    assert(task.getTags().size() == 2);
    assert(task.hasTag("学习"));
    assert(task.hasTag("重要"));
    assert(!task.hasTag("娱乐"));

    // 重复添加不增加
    task.addTag("学习");
    assert(task.getTags().size() == 2);

    // 删除标签
    task.removeTag("学习");
    assert(task.getTags().size() == 1);
    assert(!task.hasTag("学习"));
    assert(task.hasTag("重要"));

    std::cout << "    ✓ 通过" << std::endl;
}

void test_task_completed() {
    std::cout << "  [测试] Task 完成状态..." << std::endl;

    Task task("test");
    assert(!task.isCompleted());

    task.setCompleted(true);
    assert(task.isCompleted());

    task.toggleCompleted();
    assert(!task.isCompleted());

    task.toggleCompleted();
    assert(task.isCompleted());

    std::cout << "    ✓ 通过" << std::endl;
}

void test_task_serialization() {
    std::cout << "  [测试] Task 序列化/反序列化..." << std::endl;

    Task original("写作业");
    original.setId(42);
    original.setDescription("数学第三章");
    original.setPriority(Priority::High);
    original.setDueDate("2024-12-31");
    original.setCompleted(true);
    original.addTag("学习");
    original.addTag("重要");

    // 序列化
    std::string serialized = original.serialize();
    assert(!serialized.empty());

    // 反序列化
    Task restored;
    bool ok = Task::deserialize(serialized, restored);
    assert(ok);

    // 验证字段
    assert(restored.getId() == 42);
    assert(restored.getTitle() == "写作业");
    assert(restored.getDescription() == "数学第三章");
    assert(restored.getPriority() == Priority::High);
    assert(restored.getDueDate() == "2024-12-31");
    assert(restored.isCompleted());
    assert(restored.hasTag("学习"));
    assert(restored.hasTag("重要"));

    std::cout << "    ✓ 通过" << std::endl;
}

// ================================================================
//  测试 TaskList 类
// ================================================================

void test_tasklist_add_delete() {
    std::cout << "  [测试] TaskList 添加/删除..." << std::endl;

    TaskList list;
    assert(list.empty());
    assert(list.size() == 0);

    // 添加任务
    Task t1("任务一");
    int id1 = list.addTask(t1);
    assert(id1 == 1);
    assert(list.size() == 1);

    Task t2("任务二");
    int id2 = list.addTask(t2);
    assert(id2 == 2);
    assert(list.size() == 2);

    // 查找
    Task* found = list.findTask(id1);
    assert(found != nullptr);
    assert(found->getTitle() == "任务一");

    // 找不存在的
    assert(list.findTask(999) == nullptr);

    // 删除
    bool deleted = list.deleteTask(id1);
    assert(deleted);
    assert(list.size() == 1);
    assert(list.findTask(id1) == nullptr);

    // 重复删除
    deleted = list.deleteTask(id1);
    assert(!deleted);

    std::cout << "    ✓ 通过" << std::endl;
}

void test_tasklist_complete() {
    std::cout << "  [测试] TaskList 完成状态切换..." << std::endl;

    TaskList list;
    int id = list.addTask(Task("测试任务"));

    // 默认未完成
    const Task* t = list.findTask(id);
    assert(t != nullptr);
    assert(!t->isCompleted());

    // 标记完成
    bool ok = list.completeTask(id);
    assert(ok);
    assert(list.findTask(id)->isCompleted());

    // 取消完成
    ok = list.uncompleteTask(id);
    assert(ok);
    assert(!list.findTask(id)->isCompleted());

    // 切换
    ok = list.toggleTask(id);
    assert(ok);
    assert(list.findTask(id)->isCompleted());

    // 不存在的 ID
    ok = list.completeTask(999);
    assert(!ok);

    std::cout << "    ✓ 通过" << std::endl;
}

void test_tasklist_sort() {
    std::cout << "  [测试] TaskList 排序..." << std::endl;

    TaskList list;

    Task t1("低优先级任务");
    t1.setPriority(Priority::Low);
    list.addTask(t1);

    Task t2("高优先级任务");
    t2.setPriority(Priority::High);
    list.addTask(t2);

    Task t3("中优先级任务");
    t3.setPriority(Priority::Medium);
    list.addTask(t3);

    // 按优先级排序（高→低）
    auto sorted = list.sorted(TaskList::sortByPriority());
    assert(sorted.size() == 3);
    assert(sorted[0].getPriority() == Priority::High);
    assert(sorted[1].getPriority() == Priority::Medium);
    assert(sorted[2].getPriority() == Priority::Low);

    // 原列表不变
    assert(list.size() == 3);

    std::cout << "    ✓ 通过" << std::endl;
}

void test_tasklist_filter() {
    std::cout << "  [测试] TaskList 筛选..." << std::endl;

    TaskList list;

    Task t1("任务一");
    t1.addTag("学习");
    t1.setCompleted(false);
    list.addTask(t1);

    Task t2("任务二");
    t2.addTag("工作");
    t2.setCompleted(true);
    list.addTask(t2);

    Task t3("任务三");
    t3.addTag("学习");
    t3.setCompleted(true);
    list.addTask(t3);

    // 按标签筛选
    auto study_tasks = list.filtered(TaskList::filterByTag("学习"));
    assert(study_tasks.size() == 2);

    auto work_tasks = list.filtered(TaskList::filterByTag("工作"));
    assert(work_tasks.size() == 1);

    // 按完成状态筛选
    auto completed = list.filtered(TaskList::filterByCompleted(true));
    assert(completed.size() == 2);

    auto pending = list.filtered(TaskList::filterByCompleted(false));
    assert(pending.size() == 1);

    std::cout << "    ✓ 通过" << std::endl;
}

// ================================================================
//  主函数：运行所有测试
// ================================================================

int main() {
    std::cout << "================================================================" << std::endl;
    std::cout << "  运行单元测试" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << std::endl;

    try {
        // Task 类测试
        std::cout << "--- Task 类 ---" << std::endl;
        test_task_basic();
        test_task_priority();
        test_task_tags();
        test_task_completed();
        test_task_serialization();
        std::cout << std::endl;

        // TaskList 类测试
        std::cout << "--- TaskList 类 ---" << std::endl;
        test_tasklist_add_delete();
        test_tasklist_complete();
        test_tasklist_sort();
        test_tasklist_filter();
        std::cout << std::endl;

        std::cout << "================================================================" << std::endl;
        std::cout << "  ✓ 所有测试通过！" << std::endl;
        std::cout << "================================================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ 测试失败，异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
