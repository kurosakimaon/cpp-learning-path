// ================================================================
//  main.cpp —— 程序入口
//  组装各个模块，启动应用
// ================================================================
//
//  徒弟啊，main 函数是程序的入口。
//  它的职责很简单：
//    1. 创建各个对象
//    2. 把它们组装到一起
//    3. 启动主循环
//
//  注意 main 函数里的逻辑要尽量少，
//  真正的业务逻辑都应该放到各个类里去。
//  main 只负责"组装"和"启动"。
//
//  这就是所谓的"组合根"（Composition Root）模式——
//  所有对象的创建和依赖注入都集中在一个地方。
//

#include <iostream>
#include <memory>   // unique_ptr, make_unique

#include "Task.h"
#include "TaskList.h"
#include "Storage.h"
#include "FileStorage.h"
#include "ConsoleUI.h"

using namespace todo;

int main() {
    // ---- 1. 创建业务对象 ----

    // 任务列表（核心数据）
    TaskList tasks;

    // 文件存储（数据持久化）
    // 文件名就叫 todos.txt，放在当前目录
    FileStorage storage("todos.txt");

    // 控制台 UI
    ConsoleUI ui;

    // ---- 2. 启动时加载数据 ----
    //  程序一启动就从文件读取已有的任务

    std::vector<Task> loaded_tasks;
    if (storage.load(loaded_tasks)) {
        // 把加载的任务加进去
        // 注意：因为 addTask 会重新分配 ID，这里我们直接操作
        // 实际项目中应该给 TaskList 加批量添加的方法
        for (const auto& task : loaded_tasks) {
            tasks.addTask(task);
            // （ID 会被重新分配，这是简化版本的小问题）
            // 优化思路：给 TaskList 加一个 importTasks 方法
        }
    }

    // ---- 3. 启动 UI 主循环 ----

    ui.run(tasks, storage);

    // ---- 4. 退出 ----
    //  退出前保存由 ExitCommand 负责
    //  这里不用做什么

    return 0;
}
