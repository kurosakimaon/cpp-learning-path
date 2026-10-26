// ================================================================
//  Storage.h —— 持久化接口（抽象基类）
//  定义任务数据的存储接口
// ================================================================
//
//  徒弟啊，这里我们用"面向接口编程"的思想。
//
//  为什么要搞一个抽象基类 Storage 呢？
//  直接写 FileStorage 不行吗？
//
//  答案是：解耦。
//
//  上层代码（TaskList、Command）只关心"能保存、能加载"，
//  不关心数据是存在文件里、数据库里、还是云端。
//
//  有了这个抽象接口：
//    - 现在用文件存储（FileStorage）
//    - 以后想换成数据库，只要写个 DatabaseStorage 就行
//    - 上层代码一行都不用改
//
//  这就是面向接口编程的好处：
//  依赖抽象，不依赖具体实现。
//

#ifndef TODO_CLI_STORAGE_H
#define TODO_CLI_STORAGE_H

#include <vector>
#include "Task.h"

namespace todo {

class Storage {
public:
    // 虚析构函数！
    // 作为基类，析构函数必须是虚的，
    // 否则通过基类指针删除派生类对象时，
    // 派生类的析构函数不会被调用 → 内存泄漏
    virtual ~Storage() = default;

    // 保存任务列表
    // 参数是 const 引用：不修改任务列表，只读
    virtual bool save(const std::vector<Task>& tasks) = 0;

    // 加载任务列表
    // 参数是输出参数：把加载到的任务填进去
    // 返回值：是否加载成功
    virtual bool load(std::vector<Task>& out_tasks) = 0;

    // 数据源当前是否存在。
    // 用来区分"首次运行，还没有文件"和"显式 load 时文件已被删除"：
    // 前者可以当作空列表，后者不应该悄悄清空内存中的任务。
    virtual bool exists() const = 0;
};

} // namespace todo

#endif // TODO_CLI_STORAGE_H
