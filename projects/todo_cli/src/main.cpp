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
    //  首次运行时文件还不存在，这是正常的空列表；
    //  如果文件存在却读不出来，必须停止启动，绝不能带着空列表继续，
    //  否则退出时的保存会把原文件覆盖掉。

    if (storage.exists()) {
        std::vector<Task> loaded_tasks;
        if (!storage.load(loaded_tasks)) {
            std::cerr << "启动失败：无法读取 " << storage.getFilename()
                      << "。为避免覆盖原数据，程序将退出。" << std::endl;
            return 1;
        }
        // 导入时保留原 ID，保证重启后列表里的 ID 与文件一致
        tasks.importTasks(loaded_tasks);
    }

    // ---- 3. 启动 UI 主循环 ----

    ui.run(tasks, storage);

    // ---- 4. 退出 ----
    //  add/delete/complete 已经各自自动保存，退出时不再重写文件。
    //  这样只读查看后退出，不会因为文件只读而误报“保存失败”。

    return 0;
}
