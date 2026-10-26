/**
 * Boost.SmartPtr 智能指针深入用法 Demo
 *
 * 知识点覆盖：
 *   1. scoped_ptr / shared_ptr / weak_ptr / intrusive_ptr 四大智能指针对比
 *   2. 自定义删除器（deleter）的多种写法
 *   3. 循环引用问题及 weak_ptr 解决方案
 *   4. enable_shared_from_this 的正确使用场景
 *   5. 线程安全性分析（引用计数原子性 vs 对象本身线程安全）
 *   6. 与 C++11 std::shared_ptr / std::unique_ptr 的对比
 *
 * Boost 优势：
 *   - Boost.SmartPtr 中的 shared_ptr/weak_ptr 是 C++11 标准的原型；
 *     unique_ptr 则是标准库自己的设计，并非直接来自 Boost
 *   - 提供 intrusive_ptr 等标准库没有的智能指针
 *   - 支持更丰富的删除器语义和数组支持
 *   - 与 Boost 其他库（如 Boost.Asio、Boost.Serialization）深度集成
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 smart_ptr.cpp -o smart_ptr   （智能指针库是头文件实现）
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cassert>
#include <memory>

// ========== Boost 头文件说明 ==========
// <boost/scoped_ptr.hpp>     - scoped_ptr：独占所有权，不可复制，不可转移（C++11 前的 unique_ptr 替代品）
// <boost/shared_ptr.hpp>     - shared_ptr：引用计数共享所有权
// <boost/weak_ptr.hpp>       - weak_ptr：不拥有所有权的观察者，打破循环引用
// <boost/intrusive_ptr.hpp>  - intrusive_ptr：侵入式引用计数，需对象自己维护引用计数
// <boost/enable_shared_from_this.hpp> - enable_shared_from_this：在类内部获取自身的 shared_ptr
// <boost/make_shared.hpp>    - make_shared：高效创建 shared_ptr（一次内存分配）
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>

// ============================================================
// 一、自定义删除器（Custom Deleter）
// ============================================================
// Boost.shared_ptr 的删除器是类型擦除的（type-erased），
// 即删除器类型不体现在 shared_ptr 的模板参数中，
// 这是相比 std::unique_ptr 的一个设计差异（unique_ptr 的删除器是模板参数）。

// 1. 函数指针作为删除器
void file_closer(FILE* f) {
    if (f) {
        std::cout << "  [deleter] 关闭文件指针" << std::endl;
        fclose(f);
    }
}

// 2. Lambda / 函数对象作为删除器
struct ArrayDeleter {
    void operator()(int* p) const {
        std::cout << "  [deleter] 释放数组内存" << std::endl;
        delete[] p;
    }
};

// 3. 带状态的删除器（有数据成员的函数对象）
class LoggingDeleter {
public:
    explicit LoggingDeleter(const std::string& name) : name_(name), count_(0) {}

    template <typename T>
    void operator()(T* p) {
        ++count_;
        std::cout << "  [deleter] LoggingDeleter(" << name_
                  << ") 第" << count_ << "次删除对象" << std::endl;
        delete p;
    }

private:
    std::string name_;
    int count_;
};

// ============================================================
// 二、intrusive_ptr 示例（侵入式引用计数）
// ============================================================
// intrusive_ptr 要求被管理的对象自己维护引用计数，
// 适合：需要与 C 接口互操作、对象已有引用计数、嵌入式开发等场景。
// 需要全局函数 intrusive_ptr_add_ref 和 intrusive_ptr_release。

class RefCounted {
public:
    RefCounted() : ref_count_(0) {
        std::cout << "  [intrusive] RefCounted 构造，引用计数初始为 0" << std::endl;
    }

    virtual ~RefCounted() {
        std::cout << "  [intrusive] RefCounted 析构，最终引用计数 = " << ref_count_ << std::endl;
    }

    // 引用计数操作（通常由 intrusive_ptr 的全局函数调用）
    void add_ref() const { ++ref_count_; }
    void release() const {
        if (--ref_count_ == 0) {
            delete this;
        }
    }

    int ref_count() const { return ref_count_; }

private:
    // 注意：实际生产中应使用 std::atomic<int> 保证线程安全
    mutable int ref_count_;
};

// 必须在 boost 命名空间外（或与 RefCounted 同命名空间）提供这两个 ADL 函数
inline void intrusive_ptr_add_ref(const RefCounted* p) {
    p->add_ref();
}

inline void intrusive_ptr_release(const RefCounted* p) {
    p->release();
}

// ============================================================
// 三、循环引用问题与 weak_ptr 解决方案
// ============================================================
// 经典问题：两个对象互相持有对方的 shared_ptr，导致引用计数永远不为0，内存泄漏。
// 解决方案：其中一方使用 weak_ptr（观察者指针，不增加引用计数）。

// 有循环引用问题的类（BadCase）
struct BadNode {
    std::string name;
    boost::shared_ptr<BadNode> next;
    ~BadNode() { std::cout << "  [循环引用] BadNode " << name << " 析构（不会被调用！）" << std::endl; }
};

// 正确使用 weak_ptr 打破循环的类（GoodCase）
struct GoodNode {
    std::string name;
    boost::weak_ptr<GoodNode> next;  // 使用 weak_ptr，不增加引用计数
    ~GoodNode() { std::cout << "  [weak_ptr] GoodNode " << name << " 析构" << std::endl; }
};

// ============================================================
// 四、enable_shared_from_this
// ============================================================
// 场景：当一个对象已经被 shared_ptr 管理时，
// 如果在类的成员函数中需要返回指向自身的 shared_ptr，
// 直接使用 shared_ptr<T>(this) 会导致重复析构（两个独立的引用计数）。
// 正确做法：继承 enable_shared_from_this<T>，调用 shared_from_this()。

class Session : public boost::enable_shared_from_this<Session> {
public:
    Session(int id) : id_(id) {
        std::cout << "  [enable_shared_from_this] Session " << id_ << " 构造" << std::endl;
    }

    ~Session() {
        std::cout << "  [enable_shared_from_this] Session " << id_ << " 析构" << std::endl;
    }

    // 安全地获取自身的 shared_ptr
    boost::shared_ptr<Session> get_self() {
        // 正确：使用 shared_from_this()，与外部 shared_ptr 共享引用计数
        return shared_from_this();
    }

    // 错误示范（注释掉的代码）：
    // boost::shared_ptr<Session> get_self_bad() {
    //     return boost::shared_ptr<Session>(this); // 错误！会导致双重释放
    // }

    int id() const { return id_; }

private:
    int id_;
};

// ============================================================
// 五、线程安全性分析
// ============================================================
// Boost.shared_ptr 的线程安全保证：
//   1. 引用计数的增减是原子操作（线程安全）
//   2. 不同线程可以同时操作不同的 shared_ptr 实例（即使指向同一对象）
//   3. 同一个 shared_ptr 实例被多线程同时读写不是线程安全的（需要外部同步）
//   4. 被管理对象本身的线程安全需要用户自己保证

class ThreadSafeCounter {
public:
    ThreadSafeCounter() : value_(0) {}

    void increment() {
        std::lock_guard<std::mutex> lock(mtx_);
        ++value_;
    }

    int value() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return value_;
    }

private:
    mutable std::mutex mtx_;
    int value_;
};

// 验证 shared_ptr 引用计数的线程安全性
void thread_worker(boost::shared_ptr<ThreadSafeCounter> counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter->increment();
        // 每次循环创建一个临时 shared_ptr，验证引用计数操作的原子性
        boost::shared_ptr<ThreadSafeCounter> local_copy = counter;
        // local_copy 离开作用域时引用计数减1
    }
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.SmartPtr 智能指针深入用法 Demo ==========" << std::endl;

    // --------------------------------------------------------
    // 1. scoped_ptr 演示
    // --------------------------------------------------------
    // scoped_ptr 是独占所有权的智能指针，不可复制、不可转移。
    // C++11 之后被 std::unique_ptr 取代（unique_ptr 还支持移动语义和自定义删除器）。
    // Boost 优势：在 C++03 环境下提供 RAII 语义，简洁高效。
    std::cout << "\n--- 1. scoped_ptr 独占所有权 ---" << std::endl;
    {
        boost::scoped_ptr<std::string> sp(new std::string("Hello, Boost!"));
        std::cout << "  scoped_ptr 指向的值: " << *sp << std::endl;
        std::cout << "  scoped_ptr 大小: " << sizeof(sp) << " 字节（通常等于裸指针大小）" << std::endl;
        // scoped_ptr 不允许复制，离开作用域自动释放
        // boost::scoped_ptr<std::string> sp2 = sp; // 编译错误
    }
    std::cout << "  scoped_ptr 已释放资源" << std::endl;

    // --------------------------------------------------------
    // 2. shared_ptr + 自定义删除器
    // --------------------------------------------------------
    std::cout << "\n--- 2. shared_ptr 自定义删除器 ---" << std::endl;
    {
        // 2.1 函数指针删除器：管理 FILE*
        std::cout << "2.1 函数指针删除器 (FILE*):" << std::endl;
        boost::shared_ptr<FILE> fp(tmpfile(), file_closer);
        if (fp) {
            fprintf(fp.get(), "test data");
            std::cout << "  文件已写入" << std::endl;
        }

        // 2.2 函数对象删除器：管理数组
        std::cout << "2.2 函数对象删除器 (数组):" << std::endl;
        boost::shared_ptr<int> arr(new int[10], ArrayDeleter());
        arr.get()[0] = 42;
        std::cout << "  数组首元素: " << arr.get()[0] << std::endl;

        // 2.3 带状态的删除器
        std::cout << "2.3 带状态的删除器 (LoggingDeleter):" << std::endl;
        LoggingDeleter logger("my_logger");
        boost::shared_ptr<std::string> sp(new std::string("test"), logger);
        std::cout << "  字符串内容: " << *sp << std::endl;

        // 2.4 make_shared：一次内存分配，更高效
        std::cout << "2.4 make_shared 高效创建:" << std::endl;
        auto sp2 = boost::make_shared<std::string>("make_shared demo");
        std::cout << "  use_count = " << sp2.use_count()
                  << ", value = " << *sp2 << std::endl;
    }

    // --------------------------------------------------------
    // 3. weak_ptr 与循环引用
    // --------------------------------------------------------
    std::cout << "\n--- 3. weak_ptr 打破循环引用 ---" << std::endl;
    {
        // 3.1 错误示范：循环引用导致内存泄漏
        std::cout << "3.1 循环引用（BadCase - 会泄漏）:" << std::endl;
        {
            boost::shared_ptr<BadNode> a(new BadNode{"A", nullptr});
            boost::shared_ptr<BadNode> b(new BadNode{"B", nullptr});
            a->next = b;  // a 引用 b (ref_count(b) = 2)
            b->next = a;  // b 引用 a (ref_count(a) = 2)
            // 离开作用域时：a 析构 -> ref_count(a) = 1, b 析构 -> ref_count(b) = 1
            // 两个对象都不会被释放！（循环引用）
            std::cout << "  离开作用域，但 BadNode A 和 B 都不会析构（内存泄漏）" << std::endl;
        }

        // 3.2 正确示范：使用 weak_ptr
        std::cout << "3.2 使用 weak_ptr（GoodCase - 正确释放）:" << std::endl;
        {
            boost::shared_ptr<GoodNode> a(new GoodNode{"A", boost::weak_ptr<GoodNode>()});
            boost::shared_ptr<GoodNode> b(new GoodNode{"B", boost::weak_ptr<GoodNode>()});
            a->next = b;  // weak_ptr 不增加引用计数
            b->next = a;  // weak_ptr 不增加引用计数

            // 使用 weak_ptr 时需要 lock() 升级为 shared_ptr
            boost::shared_ptr<GoodNode> b_from_a = a->next.lock();
            if (b_from_a) {
                std::cout << "  通过 a->next.lock() 访问到: " << b_from_a->name << std::endl;
                std::cout << "  此时 b 的 use_count = " << b_from_a.use_count() << std::endl;
            }

            // expired() 检查对象是否已被释放
            std::cout << "  a->next 是否有效: " << (a->next.expired() ? "否" : "是") << std::endl;
        }
    }

    // --------------------------------------------------------
    // 4. intrusive_ptr 侵入式引用计数
    // --------------------------------------------------------
    std::cout << "\n--- 4. intrusive_ptr 侵入式引用计数 ---" << std::endl;
    {
        std::cout << "4.1 基本用法:" << std::endl;
        boost::intrusive_ptr<RefCounted> p1(new RefCounted());
        std::cout << "  p1 创建后引用计数: " << p1->ref_count() << std::endl;

        {
            boost::intrusive_ptr<RefCounted> p2 = p1;  // 拷贝构造
            std::cout << "  p2 拷贝后引用计数: " << p1->ref_count() << std::endl;

            boost::intrusive_ptr<RefCounted> p3;
            p3 = p1;  // 拷贝赋值
            std::cout << "  p3 赋值后引用计数: " << p1->ref_count() << std::endl;
        }
        std::cout << "  p2/p3 释放后引用计数: " << p1->ref_count() << std::endl;

        // intrusive_ptr 可以从裸指针安全构造：它直接复用对象内部的引用计数
        std::cout << "4.2 从裸指针构造（intrusive_ptr 共享已有引用计数）:" << std::endl;
        RefCounted* raw = p1.get();
        boost::intrusive_ptr<RefCounted> p4(raw);  // 安全：共享同一个引用计数
        std::cout << "  p4 从裸指针构造后引用计数: " << p1->ref_count() << std::endl;
        // 注意：shared_ptr 也能从裸指针构造，但如果这块内存已经被另一个
        // shared_ptr 管理，就会产生两套独立的引用计数，最终双重释放。
    }

    // --------------------------------------------------------
    // 5. enable_shared_from_this
    // --------------------------------------------------------
    std::cout << "\n--- 5. enable_shared_from_this ---" << std::endl;
    {
        boost::shared_ptr<Session> s1(new Session(100));
        std::cout << "  s1.use_count = " << s1.use_count() << std::endl;

        // 通过成员函数获取自身的 shared_ptr
        boost::shared_ptr<Session> s2 = s1->get_self();
        std::cout << "  get_self() 后 s1.use_count = " << s1.use_count() << std::endl;
        std::cout << "  s1 和 s2 是否指向同一对象: "
                  << (s1.get() == s2.get() ? "是" : "否") << std::endl;
        std::cout << "  s1 和 s2 是否共享所有权: "
                  << (s1.use_count() == s2.use_count() ? "是（共享）" : "否") << std::endl;

        // 注意：对象必须已经被 shared_ptr 管理才能调用 shared_from_this()
        // 否则 shared_from_this() 会抛出 boost::bad_weak_ptr 异常
    }

    // --------------------------------------------------------
    // 6. 线程安全性
    // --------------------------------------------------------
    std::cout << "\n--- 6. 线程安全性分析 ---" << std::endl;
    {
        auto counter = boost::make_shared<ThreadSafeCounter>();
        const int num_threads = 4;
        const int iterations = 10000;

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(thread_worker, counter, iterations);
        }
        for (auto& t : threads) {
            t.join();
        }

        std::cout << "  4 个线程各增加 " << iterations << " 次" << std::endl;
        std::cout << "  最终计数器值: " << counter->value()
                  << " (预期: " << num_threads * iterations << ")" << std::endl;
        std::cout << "  counter.use_count = " << counter.use_count()
                  << " (引用计数增减是原子的，线程安全)" << std::endl;
    }

    // --------------------------------------------------------
    // 7. Boost vs std 对比总结
    // --------------------------------------------------------
    std::cout << "\n--- 7. Boost.SmartPtr vs C++标准库对比 ---" << std::endl;
    std::cout << "  boost::scoped_ptr   -> std::unique_ptr (C++11，功能更强)" << std::endl;
    std::cout << "  boost::shared_ptr   -> std::shared_ptr (C++11，基本一致)" << std::endl;
    std::cout << "  boost::weak_ptr     -> std::weak_ptr (C++11，基本一致)" << std::endl;
    std::cout << "  boost::intrusive_ptr -> 标准库无对应物 (Boost 独有)" << std::endl;
    std::cout << "  boost::enable_shared_from_this -> std::enable_shared_from_this (C++11)" << std::endl;
    std::cout << "  boost::make_shared  -> std::make_shared (C++11)" << std::endl;
    std::cout << "\n  Boost 独特优势：" << std::endl;
    std::cout << "    - intrusive_ptr 侵入式智能指针" << std::endl;
    std::cout << "    - 与 Boost.Asio / Serialization / Python 等库深度集成" << std::endl;
    std::cout << "    - C++03 环境下仍可使用（历史价值）" << std::endl;
    std::cout << "    - 更丰富的辅助工具（如 allocate_shared、static_pointer_cast 等）" << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
