// ============================================================
//  smart_ptr_deep.cpp - 智能指针深入
// ============================================================
//  徒弟啊，上一课我们初步认识了智能指针。
//  今天咱深挖一下——unique_ptr、shared_ptr、weak_ptr，
//  还有那些踩过的坑和容易犯的错。
//  智能指针是现代C++的标配，你必须把它玩得溜。
// ============================================================

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <cstdio>

using namespace std;

// ============================================================
// 一、为什么需要智能指针？
// ============================================================
// 徒弟，咱先回顾一下手动管理内存的痛点：
//
// 1. 忘了释放 → 内存泄漏
// 2. 提前释放后还在用 → 野指针/悬空指针
// 3. 重复释放 → double free，未定义行为（常见是崩溃）
// 4. 异常安全：函数中间抛异常，后面的delete执行不到
// 5. 所有权不清晰：谁创建、谁释放、什么时候释放
//
// 智能指针用RAII思想把裸指针包装起来，
// 对象生命周期结束时自动调用析构函数释放内存。
//
// C++11标准库提供三种智能指针：
//   - unique_ptr：独占所有权，轻量高效
//   - shared_ptr：共享所有权，引用计数
//   - weak_ptr：配合shared_ptr，不增加引用计数
//
// C++98里的 auto_ptr 已经被废弃了，别用！
// ============================================================

void whySmartPtr()
{
    cout << "============================================================" << endl;
    cout << "  一、为什么需要智能指针？" << endl;
    cout << "============================================================" << endl;
    cout << endl;
    cout << "  手动管理内存的痛点：" << endl;
    cout << "    1. 忘记delete → 内存泄漏" << endl;
    cout << "    2. 重复delete → double free（未定义行为）" << endl;
    cout << "    3. 异常安全问题 → 抛异常后delete执行不到" << endl;
    cout << "    4. 所有权不清晰 → 谁来释放？什么时候释放？" << endl;
    cout << endl;
    cout << "  智能指针的解决方案：RAII" << endl;
    cout << "    构造时接管资源，析构时自动释放" << endl;
    cout << "    不管函数怎么return、怎么抛异常，都能正确释放" << endl;
    cout << endl;
    cout << "  三种智能指针：" << endl;
    cout << "    unique_ptr  → 独占所有权，轻量高效（首选）" << endl;
    cout << "    shared_ptr  → 共享所有权，引用计数（需要共享才用）" << endl;
    cout << "    weak_ptr    → 弱引用，配合shared_ptr解决循环引用" << endl;
}

// ============================================================
// 二、unique_ptr 详解
// ============================================================
// unique_ptr 独占它指向的对象，不能拷贝，只能移动。
// 出作用域自动delete，开销几乎和裸指针一样。
//
// 核心特点：
//   - 独占所有权：同一时间只有一个unique_ptr指向对象
//   - 不可拷贝，只能移动（move）
//   - 轻量高效：没有引用计数开销，大小等于裸指针
//   - 支持自定义删除器
//   - 可以指向数组（unique_ptr<T[]>）
//
// 常用操作：
//   - get()：获取裸指针（不释放所有权）
//   - reset()：释放当前对象，接管新对象
//   - release()：释放所有权，返回裸指针（要记得自己delete！）
//   - move()：转移所有权
// ============================================================

// 一个用来观察构造析构的类
class DemoClass
{
public:
    DemoClass(int id = 0) : m_id(id)
    {
        cout << "  [DemoClass] 构造 id=" << m_id << endl;
    }
    ~DemoClass()
    {
        cout << "  [DemoClass] 析构 id=" << m_id << endl;
    }
    void sayHello() const
    {
        cout << "  你好，我是 DemoClass " << m_id << endl;
    }
    int id() const { return m_id; }
private:
    int m_id;
};

void uniquePtr_basic()
{
    cout << "\n=== 2.1 unique_ptr 基础 ===" << endl;

    // C++14 起推荐用 make_unique（异常安全、更简洁）
    cout << "  创建 unique_ptr：" << endl;
    auto p1 = make_unique<DemoClass>(1);
    p1->sayHello();

    // 也可以直接用 new 初始化（不推荐，除非有特殊需求）
    unique_ptr<DemoClass> p2(new DemoClass(2));
    p2->sayHello();

    // get() 获取裸指针
    cout << "  p1.get() = " << p1.get() << endl;

    // reset() 释放旧对象，接管新对象
    cout << "\n  reset 演示：" << endl;
    p1.reset(new DemoClass(10));  // 先构造新的10，再由 reset 析构旧的1
    p1->sayHello();

    p1.reset(nullptr);  // 显式释放
    cout << "  reset(nullptr) 后，p1 = " << p1.get() << endl;
}

void uniquePtr_move()
{
    cout << "\n=== 2.2 unique_ptr 移动语义（所有权转移）===" << endl;

    auto p1 = make_unique<DemoClass>(100);
    cout << "  p1 拥有对象，地址: " << p1.get() << endl;

    // unique_ptr<int> p2 = p1;  // 编译错误！不能拷贝
    auto p2 = move(p1);  // 可以移动，所有权转移给p2
    cout << "  移动后：" << endl;
    cout << "    p1 = " << p1.get() << "（空了）" << endl;
    cout << "    p2 = " << p2.get() << "（p2现在拥有对象）" << endl;
    p2->sayHello();
}

void uniquePtr_release()
{
    cout << "\n=== 2.3 release() 释放所有权 ===" << endl;

    auto p1 = make_unique<DemoClass>(200);
    DemoClass* raw = p1.release();  // p1放弃所有权，返回裸指针
    cout << "  release 后，p1 = " << p1.get() << endl;
    cout << "  raw 指针指向: " << raw << endl;
    raw->sayHello();

    delete raw;  // 必须手动释放！不然就泄漏了
    cout << "  手动 delete raw 了" << endl;
}

void uniquePtr_array()
{
    cout << "\n=== 2.4 unique_ptr 管理数组 ===" << endl;

    // unique_ptr 可以直接管理数组，自动用 delete[]
    auto arr = make_unique<DemoClass[]>(3);  // 构造3个对象
    for (int i = 0; i < 3; ++i)
    {
        cout << "  arr[" << i << "].id() = " << arr[i].id() << endl;
    }
    // 出作用域自动调用 delete[]
}

// ---- 自定义删除器 ----
// 有些资源不是new出来的，不能用delete释放
// 比如fopen打开的文件要用fclose关闭
// 这时候就需要自定义删除器

struct FileCloser
{
    void operator()(FILE* f) const
    {
        if (f)
        {
            cout << "  [FileCloser] 关闭文件" << endl;
            fclose(f);
        }
    }
};

void uniquePtr_customDeleter()
{
    cout << "\n=== 2.5 unique_ptr 自定义删除器 ===" << endl;

    // 用函数对象作为删除器
    unique_ptr<FILE, FileCloser> fp(fopen("smart_ptr_test.txt", "w"));
    if (fp)
    {
        fputs("Hello unique_ptr with custom deleter!\n", fp.get());
        cout << "  文件写入成功" << endl;
    }
    // 出作用域自动调用 FileCloser()(fp) → fclose(fp)

    // 用lambda作为删除器（C++11起）
    auto deleter = [](FILE* f) {
        if (f)
        {
            cout << "  [Lambda删除器] 关闭文件" << endl;
            fclose(f);
        }
    };
    unique_ptr<FILE, decltype(deleter)> fp2(
        fopen("smart_ptr_test2.txt", "w"), deleter);
    if (fp2)
    {
        fputs("Hello from lambda deleter!\n", fp2.get());
        cout << "  第二个文件也写入成功" << endl;
    }
}

void uniquePtr_container()
{
    cout << "\n=== 2.6 unique_ptr 放容器里 ===" << endl;

    vector<unique_ptr<DemoClass>> vec;

    // 直接放进去（移动）
    vec.push_back(make_unique<DemoClass>(1));
    vec.push_back(make_unique<DemoClass>(2));
    vec.push_back(make_unique<DemoClass>(3));

    cout << "  vector 里有 " << vec.size() << " 个对象" << endl;

    // 访问
    for (const auto& p : vec)
    {
        p->sayHello();
    }

    // 容器销毁时，每个unique_ptr都会自动析构
}

// ============================================================
// 三、shared_ptr 详解
// ============================================================
// shared_ptr 允许多个指针共享同一个对象，用引用计数管理生命周期。
// 最后一个shared_ptr销毁时，对象才被释放。
//
// 核心特点：
//   - 共享所有权：多个shared_ptr可以指向同一个对象
//   - 引用计数：拷贝+1，析构-1，归零才释放
//   - 开销比unique_ptr大（两个指针：对象+控制块）
//   - 支持自定义删除器
//   - 线程安全：引用计数的增减是原子操作
//
// 控制块（control block）：
//   每个shared_ptr管理的对象都有一个控制块，里面存着：
//   - 引用计数（shared count）
//   - 弱引用计数（weak count）
//   - 删除器
//   - 分配器等
//
// make_shared：一次性分配对象+控制块的内存，更高效，异常安全
// ============================================================

void sharedPtr_basic()
{
    cout << "\n=== 3.1 shared_ptr 基础 ===" << endl;

    // 推荐用 make_shared（一次分配、异常安全）
    auto sp1 = make_shared<DemoClass>(1);
    cout << "  sp1 引用计数: " << sp1.use_count() << endl;

    {
        auto sp2 = sp1;  // 拷贝，引用计数+1
        cout << "  sp2 拷贝后，引用计数: " << sp1.use_count() << endl;

        auto sp3 = sp1;
        cout << "  sp3 拷贝后，引用计数: " << sp1.use_count() << endl;

        // sp3 离开作用域，引用计数-1
    }
    cout << "  出内层作用域后，引用计数: " << sp1.use_count() << endl;

    // reset
    sp1.reset();  // 引用计数归零，对象释放
    cout << "  reset 后，引用计数: " << sp1.use_count() << endl;
}

void sharedPtr_controlBlock()
{
    cout << "\n=== 3.2 控制块与 make_shared ===" << endl;

    // 方式1：用new创建，两次内存分配（对象 + 控制块）
    shared_ptr<DemoClass> sp1(new DemoClass(10));
    cout << "  new + shared_ptr：两次内存分配" << endl;

    // 方式2：make_shared，一次分配（对象和控制块挨在一起）
    auto sp2 = make_shared<DemoClass>(20);
    cout << "  make_shared：一次内存分配，更高效" << endl;

    // make_shared 的缺点：
    //  - 不能自定义删除器
    //  - 如果有weak_ptr存活，对象和控制块一起不释放
    //    （因为它们在同一块内存里）

    cout << "\n  shared_ptr 的大小: " << sizeof(sp2) << " 字节" << endl;
    cout << "  （两个指针：一个指向对象，一个指向控制块）" << endl;
}

void sharedPtr_customDeleter()
{
    cout << "\n=== 3.3 shared_ptr 自定义删除器 ===" << endl;

    // shared_ptr 的删除器是类型擦除的，不影响指针类型
    // 这是它和 unique_ptr 的一个重要区别
    shared_ptr<FILE> sp(fopen("shared_ptr_file.txt", "w"),
                        [](FILE* f) {
                            cout << "  [shared_ptr删除器] 关闭文件" << endl;
                            if (f) fclose(f);
                        });
    if (sp)
    {
        fputs("Hello shared_ptr!\n", sp.get());
        cout << "  文件写入成功" << endl;
    }
    // 类型是 shared_ptr<FILE>，删除器类型不体现在模板参数里
}

// ---- shared_ptr 的线程安全 ----
// 注意：引用计数的增减是原子的、线程安全的
// 但 shared_ptr 对象本身不是线程安全的！
// 多个线程同时读写同一个shared_ptr对象，需要加锁

mutex g_mutex;

void sharedPtr_threadSafety()
{
    cout << "\n=== 3.4 shared_ptr 线程安全 ===" << endl;

    auto sp = make_shared<int>(0);

    cout << "  shared_ptr 引用计数的增减是原子操作（线程安全）" << endl;
    cout << "  但 shared_ptr 对象本身不是线程安全的" << endl;
    cout << "  多个线程同时修改同一个 shared_ptr 对象，需要加锁" << endl;

    // 演示：多线程拷贝shared_ptr（安全，因为每个线程操作自己的副本）
    thread t1([sp]() {  // 值拷贝，每个线程有自己的shared_ptr
        // 只读取指向的内容，如果内容也会被改，那也要加锁
        lock_guard<mutex> lk(g_mutex);
        cout << "  线程1: use_count = " << sp.use_count() << endl;
    });
    thread t2([sp]() {
        lock_guard<mutex> lk(g_mutex);
        cout << "  线程2: use_count = " << sp.use_count() << endl;
    });
    t1.join();
    t2.join();

    cout << "  回到主线程，use_count = " << sp.use_count() << endl;
}

// ============================================================
// 四、weak_ptr 详解
// ============================================================
// weak_ptr 是 shared_ptr 的好搭档，它不增加引用计数。
// 主要用途：
//   1. 解决循环引用问题
//   2. 观察者模式：只观察对象，不影响生命周期
//
// weak_ptr 不能直接访问对象，必须先 lock() 转成 shared_ptr。
// lock() 可能返回空的 shared_ptr（如果对象已经被释放了）。
//
// expired()：检查对象是否已经被释放
// lock()：获取 shared_ptr（成功则引用计数+1）
// ============================================================

void weakPtr_basic()
{
    cout << "\n=== 4.1 weak_ptr 基础 ===" << endl;

    auto sp = make_shared<DemoClass>(1);
    cout << "  创建 shared_ptr，引用计数: " << sp.use_count() << endl;

    weak_ptr<DemoClass> wp = sp;  // weak_ptr 不增加引用计数
    cout << "  创建 weak_ptr 后，引用计数: " << sp.use_count() << endl;
    cout << "  weak_ptr expired? " << (wp.expired() ? "是" : "否") << endl;

    // 用 lock() 获取 shared_ptr
    auto sp2 = wp.lock();
    if (sp2)
    {
        cout << "  lock() 成功，引用计数: " << sp.use_count() << endl;
        sp2->sayHello();
    }

    // 释放主指针
    sp.reset();
    sp2.reset();
    cout << "  释放所有 shared_ptr 后..." << endl;
    cout << "  weak_ptr expired? " << (wp.expired() ? "是" : "否") << endl;

    auto sp3 = wp.lock();
    if (!sp3)
    {
        cout << "  lock() 失败，对象已经被释放了" << endl;
    }
}

// ---- 循环引用问题 ----
// 两个对象互相用shared_ptr指着对方，引用计数永远到不了0，内存泄漏！

class B;  // 前向声明

class A
{
public:
    shared_ptr<B> b_ptr;
    // weak_ptr<B> b_ptr;  // 解决方案：把其中一个改成weak_ptr
    A() { cout << "  [A] 构造" << endl; }
    ~A() { cout << "  [A] 析构" << endl; }
};

class B
{
public:
    shared_ptr<A> a_ptr;
    B() { cout << "  [B] 构造" << endl; }
    ~B() { cout << "  [B] 析构" << endl; }
};

void weakPtr_circularRef()
{
    cout << "\n=== 4.2 循环引用问题（weak_ptr的主要用途）===" << endl;

    cout << "  创建 A 和 B，互相引用..." << endl;
    auto a = make_shared<A>();
    auto b = make_shared<B>();

    a->b_ptr = b;  // A 指向 B
    b->a_ptr = a;  // B 指向 A

    cout << "  a 引用计数: " << a.use_count() << endl;
    cout << "  b 引用计数: " << b.use_count() << endl;

    cout << "  离开作用域..." << endl;
    cout << "  注意：你看不到析构函数被调用！因为循环引用！" << endl;
    cout << "  解决方案：把其中一个shared_ptr改成weak_ptr" << endl;
    // （此示例会泄漏内存，仅作演示用）

    // 实际项目中怎么解：
    //  1. 分析所有权关系，确定谁拥有谁
    //  2. 拥有方用 shared_ptr，被拥有方用 weak_ptr 反向引用
    //  3. 或者一方用 unique_ptr，另一方用裸指针（保证生命周期安全的前提下）
}

// ---- 观察者模式示例 ----
class Subject;  // 前向声明

class Observer
{
public:
    Observer(string name) : m_name(move(name)) {}
    void observe(shared_ptr<Subject> subj);
    void checkStatus();

private:
    string m_name;
    weak_ptr<Subject> m_subject;  // 观察者用weak_ptr，不影响生命周期
};

class Subject : public enable_shared_from_this<Subject>
{
public:
    Subject(int data) : m_data(data)
    {
        cout << "  [Subject] 构造，data=" << m_data << endl;
    }
    ~Subject() { cout << "  [Subject] 析构" << endl; }

    int data() const { return m_data; }

private:
    int m_data;
};

void Observer::observe(shared_ptr<Subject> subj)
{
    m_subject = subj;  // weak_ptr 不增加引用计数
}

void Observer::checkStatus()
{
    auto sp = m_subject.lock();
    if (sp)
    {
        cout << "  " << m_name << " 观察到 data = " << sp->data() << endl;
    }
    else
    {
        cout << "  " << m_name << " 观察到对象已经被释放了" << endl;
    }
}

void weakPtr_observer()
{
    cout << "\n=== 4.3 weak_ptr 实现观察者模式 ===" << endl;

    auto obs1 = make_shared<Observer>("观察者A");
    auto obs2 = make_shared<Observer>("观察者B");

    {
        auto subj = make_shared<Subject>(42);
        obs1->observe(subj);
        obs2->observe(subj);

        cout << "  Subject 引用计数: " << subj.use_count()
             << "（只有1个，观察者不增加计数）" << endl;

        obs1->checkStatus();
        obs2->checkStatus();

        cout << "  离开作用域，Subject 将被销毁..." << endl;
    }

    obs1->checkStatus();
    obs2->checkStatus();
}

// ============================================================
// 五、enable_shared_from_this
// ============================================================
// 问题：类的成员函数里，怎么返回指向自己的shared_ptr？
// 如果你直接 return shared_ptr<MyClass>(this);
// 那就完蛋了——会创建新的控制块，两个独立的引用计数，
// 最后两个shared_ptr都要释放同一个对象，double free！
//
// 解决方案：继承 enable_shared_from_this，用 shared_from_this()
// 它会从已有的控制块里创建shared_ptr，引用计数正确。
//
// 注意：使用 shared_from_this() 的前提是，
//       对象已经被一个shared_ptr管理了！
//       如果对象在栈上，或者用new出来还没给shared_ptr：
//       C++17 起会抛出 std::bad_weak_ptr；C++11/14 中则是未定义行为。
// ============================================================

class GoodSelf : public enable_shared_from_this<GoodSelf>
{
public:
    GoodSelf() { cout << "  [GoodSelf] 构造" << endl; }
    ~GoodSelf() { cout << "  [GoodSelf] 析构" << endl; }

    shared_ptr<GoodSelf> getSelf()
    {
        return shared_from_this();  // 正确做法
    }

    void printUseCount()
    {
        cout << "  当前引用计数: " << shared_from_this().use_count()
             << "（注意：临时shared_ptr让计数+1了）" << endl;
    }
};

class BadSelf
{
public:
    BadSelf() { cout << "  [BadSelf] 构造" << endl; }
    ~BadSelf() { cout << "  [BadSelf] 析构" << endl; }

    shared_ptr<BadSelf> getSelf()
    {
        return shared_ptr<BadSelf>(this);  // 错误！会创建新的控制块
    }
};

void enableSharedFromThis_demo()
{
    cout << "\n=== 5. enable_shared_from_this ===" << endl;

    // 正确用法
    cout << "\n  5.1 正确做法：继承 enable_shared_from_this" << endl;
    auto good = make_shared<GoodSelf>();
    cout << "  初始引用计数: " << good.use_count() << endl;

    auto good2 = good->getSelf();
    cout << "  getSelf() 后，引用计数: " << good.use_count() << endl;
    good2->printUseCount();

    // 错误用法（未定义行为，所以注释掉了）
    cout << "\n  5.2 错误做法：直接 return shared_ptr(this)" << endl;
    cout << "  后果：创建两个独立的控制块，最后 double free（未定义行为）" << endl;
    cout << "  （这个示例没运行，运行结果不可预测，常见是崩溃）" << endl;
    /*
    auto bad = make_shared<BadSelf>();
    auto bad2 = bad->getSelf();  // 两个独立的引用计数！
    // 出作用域，bad和bad2都要delete，double free！
    */

    cout << "\n  注意事项：" << endl;
    cout << "    1. 类必须 public 继承 enable_shared_from_this" << endl;
    cout << "    2. 调用 shared_from_this() 时，对象必须已被shared_ptr管理" << endl;
    cout << "    3. 不要在构造函数里调用 shared_from_this()" << endl;
    cout << "       （构造时对象还没交给shared_ptr）" << endl;
}

// ============================================================
// 六、常见误用与陷阱
// ============================================================

void commonMistakes()
{
    cout << "\n=== 6. 常见误用与陷阱 ===" << endl;

    // 6.1 裸指针和智能指针混用
    cout << "\n  6.1 裸指针和智能指针混用" << endl;
    cout << "      错误示例：" << endl;
    cout << "        int* raw = new int(10);" << endl;
    cout << "        shared_ptr<int> sp1(raw);" << endl;
    cout << "        shared_ptr<int> sp2(raw);  // 两个独立控制块！" << endl;
    cout << "      后果：double free，未定义行为（常见是崩溃）" << endl;
    cout << "      正确做法：尽量用 make_shared / make_unique" << endl;
    cout << "      不要从同一个裸指针创建多个智能指针" << endl;

    // 6.2 循环引用
    cout << "\n  6.2 循环引用" << endl;
    cout << "      现象：两个对象互相用shared_ptr指着，谁也不释放" << endl;
    cout << "      解决：分析所有权，其中一方改用weak_ptr" << endl;
    cout << "      原则：拥有用shared_ptr，观察/反向引用用weak_ptr" << endl;

    // 6.3 unique_ptr 转 shared_ptr
    cout << "\n  6.3 unique_ptr 可以转 shared_ptr，反过来不行" << endl;
    cout << "      unique_ptr<int> up = make_unique<int>(10);" << endl;
    cout << "      shared_ptr<int> sp = move(up);  // 可以" << endl;
    cout << "      因为unique_ptr是独占的，转成shared的是升级" << endl;
    cout << "      shared_ptr不能转unique_ptr（可能有多个持有者）" << endl;

    // 6.4 get() 出来的指针别乱删
    cout << "\n  6.4 get() 出来的裸指针，不要delete！" << endl;
    cout << "      智能指针还管着这块内存呢，你删了它就重复释放了" << endl;
    cout << "      get() 只是让你临时用用，所有权还是智能指针的" << endl;

    // 6.5 数组的 shared_ptr
    cout << "\n  6.5 shared_ptr 管理数组（C++17前要注意）" << endl;
    cout << "      C++17 前：shared_ptr<int> sp(new int[10]); // 错！用delete释放" << endl;
    cout << "      需要自定义删除器：shared_ptr<int> sp(new int[10], default_delete<int[]>());" << endl;
    cout << "      C++17 起：shared_ptr<int[]> sp(new int[10]); // 正确" << endl;
    cout << "      但还是推荐用 vector 代替动态数组" << endl;

    // 6.6 用智能指针管理不是new出来的资源
    cout << "\n  6.6 用智能指针管理栈对象？作死！" << endl;
    cout << "      int x = 10;" << endl;
    cout << "      shared_ptr<int> sp(&x);  // 大错特错！" << endl;
    cout << "      栈对象本来就会自动释放，智能指针再 delete 一次是未定义行为" << endl;
}

// ============================================================
// 七、选择建议与最佳实践
// ============================================================

void bestPractices()
{
    cout << "\n============================================================" << endl;
    cout << "  七、选择建议与最佳实践" << endl;
    cout << "============================================================" << endl;
    cout << endl;
    cout << "  选择原则：" << endl;
    cout << "    1. 默认用 unique_ptr" << endl;
    cout << "       轻量、高效、语义清晰（独占）" << endl;
    cout << "       需要共享的时候才用 shared_ptr" << endl;
    cout << endl;
    cout << "    2. 优先使用 make_unique / make_shared" << endl;
    cout << "       - 更简洁，不用写new" << endl;
    cout << "       - 异常安全（函数参数求值顺序问题）" << endl;
    cout << "       - make_shared 更高效（一次分配）" << endl;
    cout << endl;
    cout << "    3. 所有权设计要清晰" << endl;
    cout << "       - 谁创建、谁拥有、谁释放，都要明确" << endl;
    cout << "       - 拥有关系用 unique_ptr 或 shared_ptr" << endl;
    cout << "       - 观察关系用 raw pointer 或 weak_ptr" << endl;
    cout << "       - 函数参数：只读传引用或const指针" << endl;
    cout << endl;
    cout << "    4. 避免循环引用" << endl;
    cout << "       - 父子关系：父→子用shared_ptr，子→父用weak_ptr" << endl;
    cout << "       - 平等关系：一方拥有，另一方观察" << endl;
    cout << "       - 实在搞不定，用个管理器统一管理生命周期" << endl;
    cout << endl;
    cout << "    5. 别把智能指针当银弹" << endl;
    cout << "       - 栈上能放的就别放堆上" << endl;
    cout << "       - 容器直接存对象（小对象），比存指针快" << endl;
    cout << "       - 智能指针有开销，别滥用" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    cout << "============================================================" << endl;
    cout << "  智能指针深入 - 中级进阶Demo" << endl;
    cout << "============================================================" << endl;

    // 一、为什么需要智能指针
    whySmartPtr();

    // 二、unique_ptr
    uniquePtr_basic();
    uniquePtr_move();
    uniquePtr_release();
    uniquePtr_array();
    uniquePtr_customDeleter();
    uniquePtr_container();

    // 三、shared_ptr
    sharedPtr_basic();
    sharedPtr_controlBlock();
    sharedPtr_customDeleter();
    sharedPtr_threadSafety();

    // 四、weak_ptr
    weakPtr_basic();
    weakPtr_circularRef();
    weakPtr_observer();

    // 五、enable_shared_from_this
    enableSharedFromThis_demo();

    // 六、常见误用
    commonMistakes();

    // 七、最佳实践
    bestPractices();

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "  徒弟啊，智能指针这块儿，我跟你说几句掏心窝子的：" << endl;
    cout << endl;
    cout << "  1. 优先使用 unique_ptr，需要共享才用 shared_ptr。" << endl;
    cout << "     unique_ptr 开销小、语义清晰，是首选。" << endl;
    cout << "     别上来就 shared_ptr，共享所有权是有代价的。" << endl;
    cout << "     先想清楚谁拥有这个对象。" << endl;
    cout << endl;
    cout << "  2. 用 make_unique 和 make_shared，别自己 new。" << endl;
    cout << "     不仅写起来简洁，而且异常安全。" << endl;
    cout << "     函数参数里如果有多个new，求值顺序不确定，可能泄漏。" << endl;
    cout << "     make_* 系列就没这个问题。" << endl;
    cout << endl;
    cout << "  3. 循环引用是 shared_ptr 的大敌。" << endl;
    cout << "     用之前一定要想清楚所有权关系。" << endl;
    cout << "     父拥有子，子不能拥有父，子只能用weak_ptr看父。" << endl;
    cout << "     发现内存泄漏，先查是不是有循环引用。" << endl;
    cout << endl;
    cout << "  4. enable_shared_from_this 用对了是神器，" << endl;
    cout << "     用错了就是坑。记住前提条件：对象必须已经被shared_ptr管理。" << endl;
    cout << "     别在构造函数里调用 shared_from_this()。" << endl;
    cout << endl;
    cout << "  5. 智能指针不是万能的。" << endl;
    cout << "     它解决的是内存泄漏问题，不是所有问题。" << endl;
    cout << "     悬空引用、迭代器失效、死锁……这些它管不了。" << endl;
    cout << "     该怎么写还得怎么写，别以为有了智能指针就可以放飞自我。" << endl;
    cout << endl;
    cout << "  6. 记住一句话：所有权要清晰。" << endl;
    cout << "     每个资源都应该有明确的所有者，" << endl;
    cout << "     所有者负责生命周期，其他人只能借用。" << endl;
    cout << "     这是设计问题，不是语法问题。" << endl;
    cout << "============================================================" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习（回去自己做）" << endl;
    cout << "============================================================" << endl;
    cout << "  1. 用 unique_ptr 实现一个简单的智能指针（MyUniquePtr），" << endl;
    cout << "     支持构造、析构、移动构造、operator*、operator->、get、reset" << endl;
    cout << "  2. 设计一个简单的树结构，父节点有多个子节点，" << endl;
    cout << "     子节点能找到父节点，用正确的智能指针避免循环引用" << endl;
    cout << "  3. 用 shared_ptr 实现一个简单的引用计数智能指针（MySharedPtr）" << endl;
    cout << "  4. 思考：为什么 make_shared 不能自定义删除器？" << endl;
    cout << "  5. 思考：weak_ptr 的用途除了解决循环引用，还有什么？" << endl;
    cout << "     （提示：缓存、对象池、观察者模式...）" << endl;
    cout << "  6. 动手验证：循环引用的场景下，用工具（如valgrind或" << endl;
    cout << "     地址消毒器）检测内存泄漏" << endl;
    cout << "============================================================" << endl;

    return 0;
}
