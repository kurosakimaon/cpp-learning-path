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

// 即使以 Release（-DNDEBUG）编译，也要让 assert 生效。
// 否则断言会被预处理器全部去掉，测试永远“通过”，等于没测。
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "Task.h"
#include "TaskList.h"
#include "FileStorage.h"

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

void test_task_serialization_edge_cases() {
    std::cout << "  [测试] 序列化边界情况..." << std::endl;

    // 没有标签时，最后一个字段为空，反序列化不能把它当成“字段缺失”
    {
        Task original("无标签任务");
        original.setId(7);
        std::string line = original.serialize();

        Task restored;
        assert(Task::deserialize(line, restored));
        assert(restored.getId() == 7);
        assert(restored.getTitle() == "无标签任务");
        assert(restored.getTags().empty());
    }

    // 标题、描述、标签里出现分隔符和反斜杠，也要能原样还原
    {
        Task original("a|b,c\\d");
        original.setId(8);  // 真实任务保存时一定有正数 ID
        original.setDescription("描述|x,y\\z");
        original.addTag("tag,1");
        original.addTag("tag|2");
        original.addTag("tag\\3");

        Task restored;
        assert(Task::deserialize(original.serialize(), restored));
        assert(restored.getTitle() == "a|b,c\\d");
        assert(restored.getDescription() == "描述|x,y\\z");
        assert(restored.getTags().size() == 3);
        assert(restored.hasTag("tag,1"));
        assert(restored.hasTag("tag|2"));
        assert(restored.hasTag("tag\\3"));
    }

    // 字段数量不对、数字字段非法，都要拒绝
    {
        Task task;
        assert(!Task::deserialize("1|字段太少", task));
        assert(!Task::deserialize("abc|t|d|中||0|0|", task));
        assert(!Task::deserialize("1abc|t|d|中||0|0|", task));  // ID 后面有垃圾
        assert(!Task::deserialize("0|t|d|中||0|0|", task));     // ID 必须是正数
        assert(!Task::deserialize("-1|t|d|中||0|0|", task));
        assert(!Task::deserialize(
            std::to_string(std::numeric_limits<int>::max()) + "|t|d|中||0|0|",
            task));
        assert(!Task::deserialize("1|t|d|中||0|12x|", task));   // 时间戳后面有垃圾
    }

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

void test_tasklist_import() {
    std::cout << "  [测试] TaskList 导入保留原 ID..." << std::endl;

    // 模拟持久化文件：先有 3 个任务，用户删除了 ID=1 的任务
    TaskList source;
    source.addTask(Task("任务一"));
    int kept_id = source.addTask(Task("任务二"));
    source.addTask(Task("任务三"));
    source.deleteTask(1);

    // 导入后必须保留 2、3 这两个 ID，不能从 1 重新编号
    TaskList list;
    list.importTasks(source.getAllTasks());

    assert(list.size() == 2);
    assert(list.findTask(1) == nullptr);
    assert(list.findTask(kept_id) != nullptr);
    assert(list.findTask(kept_id)->getTitle() == "任务二");
    assert(list.findTask(3) != nullptr);

    // 新任务应接在最大 ID 之后，而不是与已有 ID 冲突
    int new_id = list.addTask(Task("新任务"));
    assert(new_id == 4);
    assert(list.getNextId() == 5);

    std::cout << "    ✓ 通过" << std::endl;
}

void test_tasklist_clear() {
    std::cout << "  [测试] TaskList 清空..." << std::endl;

    TaskList list;
    list.addTask(Task("任务一"));
    list.addTask(Task("任务二"));

    list.clear();
    assert(list.empty());
    assert(list.getNextId() == 1);

    // 清空后新任务应重新从 1 开始编号
    assert(list.addTask(Task("新任务")) == 1);

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
//  测试 FileStorage 类
// ================================================================

void test_filestorage_roundtrip() {
    std::cout << "  [测试] 文件存储保存/加载..." << std::endl;

    const std::string path = "test_todos_tmp.txt";
    std::remove(path.c_str());  // 从干净状态开始

    FileStorage storage(path);

    // 文件不存在：exists() 为 false，load 失败且不能清空调用者的数据
    assert(!storage.exists());
    std::vector<Task> loaded;
    loaded.push_back(Task("占位"));  // 确认 load 失败时输出参数保持不变
    assert(!storage.load(loaded));
    assert(loaded.size() == 1);
    assert(loaded[0].getTitle() == "占位");

    // 保存两个任务（其中一个没有标签/日期，一个有特殊字符）
    Task t1("普通任务");
    t1.setId(1);
    t1.setPriority(Priority::High);

    Task t2("带|竖线,逗号\\的任务");
    t2.setId(2);
    t2.setDescription("描述|也有,分隔符");
    t2.setDueDate("2024-12-31");
    t2.addTag("学习,重要");
    t2.addTag("a|b");

    std::vector<Task> tasks = {t1, t2};
    assert(storage.save(tasks));
    assert(storage.exists());
    // 保存后临时文件应已被改名，不能留下 .tmp 垃圾
    assert(!std::ifstream(path + ".tmp").good());

    std::vector<Task> result;
    assert(storage.load(result));
    assert(result.size() == 2);
    assert(result[0].getTitle() == "普通任务");
    assert(result[0].getTags().empty());
    assert(result[1].getTitle() == "带|竖线,逗号\\的任务");
    assert(result[1].getDescription() == "描述|也有,分隔符");
    assert(result[1].getDueDate() == "2024-12-31");
    assert(result[1].hasTag("学习,重要"));
    assert(result[1].hasTag("a|b"));

    std::remove(path.c_str());

    std::cout << "    ✓ 通过" << std::endl;
}

void test_filestorage_encoding_and_validation() {
    std::cout << "  [测试] 文件编码、坏行与重复 ID..." << std::endl;

    const std::string path = "test_todos_encoding_tmp.txt";
    std::remove(path.c_str());

    // Windows 编辑器常见的 UTF-8 BOM、CRLF 和空行都要能处理
    {
        std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
        assert(ofs.is_open());
        ofs << "\xEF\xBB\xBF"
            << "1|BOM任务|说明|高||0|1700000000|学习,重要\r\n";
        ofs << "\r\n";
        ofs << "2|普通任务||低||1|1700000001|\r\n";
    }
    {
        FileStorage storage(path);
        std::vector<Task> loaded;
        assert(storage.load(loaded));
        assert(loaded.size() == 2);
        assert(loaded[0].getTitle() == "BOM任务");
        assert(loaded[0].getPriority() == Priority::High);
        assert(loaded[0].hasTag("学习"));
        assert(loaded[0].hasTag("重要"));
        assert(loaded[1].getTitle() == "普通任务");
        assert(loaded[1].isCompleted());
    }

    // 有坏行时整体加载失败，调用者内存里的数据不能被清空
    {
        std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
        ofs << "1|好任务||中||0|1700000000|\n";
        ofs << "2|坏任务|字段不够\n";
    }
    {
        FileStorage storage(path);
        std::vector<Task> keep;
        keep.push_back(Task("内存中的任务"));
        assert(!storage.load(keep));
        assert(keep.size() == 1);
        assert(keep[0].getTitle() == "内存中的任务");
    }

    // 重复 ID 会让操作只能命中其中一个，必须拒绝
    {
        std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
        ofs << "1|任务A||中||0|1700000000|\n";
        ofs << "1|任务B||中||0|1700000001|\n";
    }
    {
        FileStorage storage(path);
        std::vector<Task> loaded;
        assert(!storage.load(loaded));
    }

    std::remove(path.c_str());
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
        test_task_serialization_edge_cases();
        std::cout << std::endl;

        // TaskList 类测试
        std::cout << "--- TaskList 类 ---" << std::endl;
        test_tasklist_add_delete();
        test_tasklist_import();
        test_tasklist_clear();
        test_tasklist_complete();
        test_tasklist_sort();
        test_tasklist_filter();
        std::cout << std::endl;

        // FileStorage 类测试
        std::cout << "--- FileStorage 类 ---" << std::endl;
        test_filestorage_roundtrip();
        test_filestorage_encoding_and_validation();
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
