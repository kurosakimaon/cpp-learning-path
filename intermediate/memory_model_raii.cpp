// ============================================================
//  memory_model_raii.cpp - 内存模型与RAII（核心重点！）
// ============================================================
//  徒弟啊，这一课是C++的命根子。不懂内存，你写的C++代码就是空中楼阁。
//  今天咱把内存分区、栈和堆、还有C++最伟大的思想——RAII，一次性讲透。
//  竖起耳朵听，这玩意儿面试必考，工作中更是天天用。
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <memory>   // 智能指针头文件
#include <cstring>
#include <mutex>

using namespace std;

// ============================================================
// 一、C++内存分区概览
// ============================================================
// 徒弟你记好了，一个程序跑起来，内存大概分成这几块：
//
// 1. 栈区（Stack）：函数局部变量、函数参数、返回地址等，自动分配释放
// 2. 堆区（Heap）：动态分配的内存，需要手动new/delete或malloc/free
// 3. 全局/静态区（Global/Static）：全局变量、静态变量，程序结束才释放
// 4. 常量区（Constant）：字符串常量、const全局变量，只读
// 5. 代码区（Code）：存放函数体的二进制代码，只读
//
// 咱一个个来拆。
// ============================================================

// ---- 全局/静态区的变量 ----
int g_globalVar = 100;        // 全局变量 → 全局/静态区（已初始化）
int g_uninitGlobal;           // 全局变量 → 全局/静态区（未初始化，默认0）
static int s_staticVar = 200; // 静态全局变量 → 全局/静态区

const int G_CONST = 300;      // 全局const → 常量区（只读）

// ============================================================
// 二、栈内存详解
// ============================================================
// 栈内存，就像一摞盘子。函数调用就往上面摞（压栈），函数结束就拿下来（弹栈）。
// 特点：
//   - 分配释放由编译器自动完成，不用你操心
//   - 速度贼快，就是移动一下栈指针
//   - 大小有限，Windows默认1MB，Linux默认8MB，搞大了就栈溢出
//   - 先进后出（LIFO）
// ============================================================

void stackDemo_basic()
{
    cout << "\n=== 2.1 栈内存基础演示 ===" << endl;

    // 这些局部变量都在栈上
    int a = 10;           // 栈上的int
    double b = 3.14;      // 栈上的double
    char str[] = "hello"; // 栈上的数组（字符串内容也在栈上）

    cout << "  局部变量 a 的地址: " << &a << " 值: " << a << endl;
    cout << "  局部变量 b 的地址: " << &b << " 值: " << b << endl;
    cout << "  栈上数组 str 地址: " << (void*)str << " 内容: " << str << endl;

    // 观察地址变化——栈是从高地址向低地址增长的
    int c = 20;
    cout << "  后定义的 c 的地址: " << &c << " (比a低，说明栈向下生长)" << endl;
}

// 递归演示栈溢出（注意：注释掉了避免真的崩掉，徒弟你懂原理就行）
/*
void stackOverflowDemo(int n)
{
    int bigArray[10000];  // 每次调用都分配一大块栈内存
    cout << "第 " << n << " 层递归" << endl;
    stackOverflowDemo(n + 1);  // 无限递归，栈迟早撑爆
}
*/

// 函数调用栈演示
int stackDemo_callChain(int x)
{
    cout << "\n=== 2.2 函数调用栈演示 ===" << endl;
    cout << "  进入 stackDemo_callChain，参数 x = " << x << endl;
    int result = x * 2;
    cout << "  计算结果 result = " << result << endl;
    cout << "  离开 stackDemo_callChain" << endl;
    return result;
}

// ============================================================
// 三、堆内存详解
// ============================================================
// 堆内存，就像一个大仓库，你要多少就自己去申请（new/malloc），
// 用完了必须自己还回去（delete/free），不还就叫内存泄漏。
//
// 特点：
//   - 空间大，理论上能用到虚拟内存上限
//   - 分配释放慢，要找空闲块，还可能产生碎片
//   - 手动管理，容易出bug
//   - 没有释放顺序限制
// ============================================================

void heapDemo_newDelete()
{
    cout << "\n=== 3.1 new/delete 演示 ===" << endl;

    // new 做了两件事：1. 分配内存  2. 调用构造函数
    int* p = new int(42);  // 在堆上分配一个int，初始化为42
    cout << "  new 出来的 int，地址: " << p << " 值: " << *p << endl;

    // delete 也做两件事：1. 调用析构函数  2. 释放内存
    delete p;
    p = nullptr;  // 好习惯：释放后置空，防止野指针
    cout << "  delete 后置空，避免野指针" << endl;

    // new 数组
    int* arr = new int[5];  // 分配5个int的数组
    for (int i = 0; i < 5; ++i)
    {
        arr[i] = i * 10;
    }
    cout << "  new[] 数组内容: ";
    for (int i = 0; i < 5; ++i)
    {
        cout << arr[i] << " ";
    }
    cout << endl;

    // 注意！数组要用 delete[]，否则行为未定义！
    delete[] arr;
    arr = nullptr;
    cout << "  数组用 delete[] 释放，别漏了方括号！" << endl;
}

void heapDemo_mallocFree()
{
    cout << "\n=== 3.2 malloc/free 演示（C风格） ===" << endl;

    // malloc 只分配内存，不调用构造函数
    int* p = (int*)malloc(sizeof(int));
    *p = 100;
    cout << "  malloc 出来的 int，值: " << *p << endl;

    // free 只释放内存，不调用析构函数
    free(p);
    p = nullptr;
    cout << "  free 释放内存" << endl;

    // 重要提醒：new/delete 会调用构造析构，malloc/free 不会
    // 对于类对象，一定要用 new/delete，不能混用 malloc/free！
    cout << "  注意：类对象必须用 new/delete，malloc/free 不会调用构造析构！" << endl;
}

// 内存碎片演示（概念性）
void heapDemo_fragmentation()
{
    cout << "\n=== 3.3 内存碎片概念演示 ===" << endl;
    cout << "  想象一下堆内存是这样的：[占用][空闲][占用][空闲小][占用]" << endl;
    cout << "  虽然总空闲空间够，但不连续，大的对象放不下——这就是碎片" << endl;
    cout << "  频繁的小内存分配释放容易产生碎片" << endl;
    cout << "  解决办法：内存池、预分配、合理的分配策略" << endl;
}

// ============================================================
// 四、栈 vs 堆 对比
// ============================================================
void stackVsHeap()
{
    cout << "\n=== 4. 栈 vs 堆 对比 ===" << endl;
    cout << "  对比项      | 栈(Stack)              | 堆(Heap)" << endl;
    cout << "  ------------|------------------------|----------------------" << endl;
    cout << "  分配方式    | 编译器自动             | 手动 new/malloc" << endl;
    cout << "  释放方式    | 函数结束自动释放       | 手动 delete/free" << endl;
    cout << "  速度        | 快（移动栈指针）       | 慢（找空闲块）" << endl;
    cout << "  空间大小    | 小（MB级别）           | 大（GB级别）" << endl;
    cout << "  碎片问题    | 无                     | 有" << endl;
    cout << "  生长方向    | 向下（高→低地址）      | 向上（低→高地址）" << endl;
    cout << "  灵活性      | 大小固定编译时确定     | 动态分配运行时确定" << endl;
    cout << endl;
    cout << "  经验法则：小数据、生命周期短 → 栈" << endl;
    cout << "           大数据、生命周期长 → 堆" << endl;
}

// ============================================================
// 五、常见内存错误（踩坑合集）
// ============================================================
void memoryErrors_demo()
{
    cout << "\n=== 5. 常见内存错误 ===" << endl;

    // --- 5.1 野指针（指向已经释放的内存或未初始化的指针）---
    cout << "\n  5.1 野指针" << endl;
    cout << "      错误示例：" << endl;
    cout << "        int* p;  // 未初始化，指向随机位置" << endl;
    cout << "        *p = 10; // 严重错误！行为未定义" << endl;
    cout << "      正确做法：指针一定要初始化，释放后置 nullptr" << endl;

    // --- 5.2 重复释放（double free）---
    cout << "\n  5.2 重复释放（double free）" << endl;
    cout << "      错误示例：" << endl;
    cout << "        int* p = new int(10);" << endl;
    cout << "        delete p;" << endl;
    cout << "        delete p;  // 同一块内存释放两次！崩溃！" << endl;
    cout << "      正确做法：释放后置 nullptr，delete nullptr 是安全的" << endl;

    // --- 5.3 内存越界 ---
    cout << "\n  5.3 内存越界（buffer overflow）" << endl;
    cout << "      错误示例：" << endl;
    cout << "        int* arr = new int[5];" << endl;
    cout << "        arr[5] = 10;  // 只有0~4，越界了！" << endl;
    cout << "      后果：可能破坏其他数据，可能崩溃，可能被黑客利用" << endl;

    // --- 5.4 内存泄漏 ---
    cout << "\n  5.4 内存泄漏（memory leak）" << endl;
    cout << "      错误示例：" << endl;
    cout << "        void func() {" << endl;
    cout << "            int* p = new int[1000];" << endl;
    cout << "            // ... 忘了 delete" << endl;
    cout << "        } // 函数结束，p没了，但内存还占着！" << endl;
    cout << "      后果：程序越跑越慢，最后OOM（内存不足）" << endl;

    // --- 5.5 返回局部变量的地址 ---
    cout << "\n  5.5 返回局部变量的地址/引用" << endl;
    cout << "      错误示例：" << endl;
    cout << "        int* badFunc() {" << endl;
    cout << "            int x = 10;" << endl;
    cout << "            return &x;  // x在栈上，函数结束就没了！" << endl;
    cout << "        }" << endl;
    cout << "      后果：返回的是悬空指针，访问就炸" << endl;
}

// ============================================================
// 六、RAII思想——C++的灵魂
// ============================================================
// 徒弟，重头戏来了！RAII 是 C++ 最伟大的思想，没有之一。
//
// RAII = Resource Acquisition Is Initialization
//        资源获取即初始化
//
// 核心思想：
//   1. 把资源（内存、文件句柄、锁、套接字...）的获取放到构造函数里
//   2. 把资源的释放放到析构函数里
//   3. 用栈上的对象管理资源，对象生命周期结束自动调用析构
//   4. 这样不管函数怎么返回（正常return、异常抛出），资源都能被正确释放
//
// 说白了就是：谁申请谁负责，用对象帮你盯着，出了作用域自动还回去。
// ============================================================

// ---- 示例1：用类管理堆内存 ----
class IntArray
{
public:
    // 构造函数：获取资源（分配内存）
    explicit IntArray(size_t size)
        : m_data(new int[size]), m_size(size)
    {
        cout << "  [IntArray] 构造函数，分配了 " << size << " 个int的内存" << endl;
    }

    // 析构函数：释放资源
    ~IntArray()
    {
        delete[] m_data;
        cout << "  [IntArray] 析构函数，释放了内存" << endl;
    }

    // 禁用拷贝构造和拷贝赋值（防止浅拷贝导致double free）
    IntArray(const IntArray&) = delete;
    IntArray& operator=(const IntArray&) = delete;

    // 移动构造（C++11起，转移资源所有权）
    IntArray(IntArray&& other) noexcept
        : m_data(other.m_data), m_size(other.m_size)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        cout << "  [IntArray] 移动构造，资源转移了" << endl;
    }

    int& operator[](size_t index)
    {
        return m_data[index];
    }

    size_t size() const { return m_size; }

private:
    int* m_data;
    size_t m_size;
};

void raiiDemo_intArray()
{
    cout << "\n=== 6.1 RAII示例：用类管理堆内存 ===" << endl;
    cout << "  进入函数..." << endl;
    {
        IntArray arr(5);  // 栈上的对象，管理着堆上的内存
        for (size_t i = 0; i < arr.size(); ++i)
        {
            arr[i] = static_cast<int>(i * 10);
        }
        cout << "  数组内容: ";
        for (size_t i = 0; i < arr.size(); ++i)
        {
            cout << arr[i] << " ";
        }
        cout << endl;
        // 出作用域，arr 的析构函数自动调用，内存自动释放
    }
    cout << "  离开作用域，内存已经自动释放了，是不是很爽？" << endl;
}

// ---- 示例2：用类管理文件句柄 ----
class FileHandler
{
public:
    explicit FileHandler(const char* filename)
        : m_file(fopen(filename, "w"))
    {
        cout << "  [FileHandler] 打开文件: " << filename << endl;
    }

    ~FileHandler()
    {
        if (m_file)
        {
            fclose(m_file);
            cout << "  [FileHandler] 关闭文件" << endl;
        }
    }

    // 禁用拷贝，允许移动
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;

    void write(const char* str)
    {
        if (m_file)
        {
            fputs(str, m_file);
        }
    }

private:
    FILE* m_file;
};

void raiiDemo_fileHandle()
{
    cout << "\n=== 6.2 RAII示例：管理文件句柄 ===" << endl;
    cout << "  进入函数..." << endl;
    {
        FileHandler fh("raii_test.txt");
        fh.write("Hello RAII!\n");
        // 即使中间return或者抛异常，文件也会被正确关闭
        // 这就是RAII的威力——异常安全！
    }
    cout << "  离开作用域，文件自动关闭了" << endl;
}

// ---- 示例3：用类管理锁 ----
class MyLock
{
public:
    explicit MyLock(mutex& mtx) : m_mtx(mtx)
    {
        m_mtx.lock();
        cout << "  [MyLock] 加锁了" << endl;
    }

    ~MyLock()
    {
        m_mtx.unlock();
        cout << "  [MyLock] 解锁了" << endl;
    }

    MyLock(const MyLock&) = delete;
    MyLock& operator=(const MyLock&) = delete;

private:
    mutex& m_mtx;
};

mutex g_mutex;  // 全局锁，仅作演示

void raiiDemo_lock()
{
    cout << "\n=== 6.3 RAII示例：管理互斥锁 ===" << endl;
    cout << "  进入临界区..." << endl;
    {
        MyLock lk(g_mutex);  // 构造时加锁
        cout << "  正在操作共享资源..." << endl;
        // 不管怎么退出，析构时都会解锁
        // （STL里的 lock_guard / unique_lock 就是这个原理）
    }
    cout << "  离开临界区，锁自动释放了" << endl;
    cout << "  （标准库的 std::lock_guard 就是这么干的）" << endl;
}

// ============================================================
// 七、智能指针初体验
// ============================================================
// 徒弟你看，自己写RAII类也不难，但每个资源都写一遍也挺烦的。
// 标准库早就给你准备好了——智能指针！
//
// unique_ptr：独占所有权，不能拷贝，只能移动
// shared_ptr：共享所有权，引用计数，大家一起用
// weak_ptr：配合shared_ptr，解决循环引用问题
//
// 咱这里先露个脸，下一课 smart_ptr_deep.cpp 专门深挖。
// ============================================================

void smartPtr_intro()
{
    cout << "\n=== 7. 智能指针初体验 ===" << endl;

    // unique_ptr：独占式智能指针
    cout << "\n  7.1 unique_ptr - 独占所有权" << endl;
    unique_ptr<int> uPtr(new int(100));
    cout << "      uPtr 指向的值: " << *uPtr << endl;
    // unique_ptr<int> uPtr2 = uPtr;  // 编译错误！不能拷贝
    unique_ptr<int> uPtr2 = move(uPtr);  // 可以移动，所有权转移
    cout << "      移动后，uPtr2 值: " << *uPtr2 << endl;
    // 出作用域自动delete，不用操心

    // shared_ptr：共享式智能指针
    cout << "\n  7.2 shared_ptr - 共享所有权（引用计数）" << endl;
    shared_ptr<int> sPtr1 = make_shared<int>(200);
    cout << "      sPtr1 引用计数: " << sPtr1.use_count() << endl;
    shared_ptr<int> sPtr2 = sPtr1;  // 拷贝，引用计数+1
    cout << "      拷贝后，引用计数: " << sPtr1.use_count() << endl;
    cout << "      两个指针指向同一个值: " << *sPtr1 << " = " << *sPtr2 << endl;
    // 最后一个shared_ptr销毁时，内存才释放

    cout << "\n  智能指针就是RAII的典型应用，把裸指针包装成对象，" << endl;
    cout << "  出作用域自动释放，再也不用担心内存泄漏了！" << endl;
}

// ============================================================
// 八、综合演示：没有RAII vs 有RAII
// ============================================================

// 没有RAII的写法——繁琐且容易出错
void noRaiiExample()
{
    cout << "\n=== 8.1 没有RAII的痛苦 ===" << endl;
    int* p1 = new int(10);
    int* p2 = new int(20);

    // 假设中间有很多逻辑...
    // 如果中间某个地方return了，后面的delete就执行不到了！
    // 如果抛异常了，也执行不到！

    cout << "  p1 = " << *p1 << ", p2 = " << *p2 << endl;

    // 必须手动释放，顺序还不能错
    delete p1;
    delete p2;
    cout << "  手动释放了两个指针，好麻烦啊..." << endl;
    cout << "  而且万一中间return或者抛异常，就泄漏了！" << endl;
}

// 有RAII的写法——优雅且安全
void withRaiiExample()
{
    cout << "\n=== 8.2 有RAII的优雅 ===" << endl;
    auto p1 = make_unique<int>(10);
    auto p2 = make_unique<int>(20);

    cout << "  p1 = " << *p1 << ", p2 = " << *p2 << endl;
    // 不用管释放，出作用域自动来
    // 不管return还是抛异常，都安全
    cout << "  不用手动释放，出作用域自动搞定！" << endl;
    cout << "  异常安全、代码简洁、心智负担小——这就是RAII的魅力" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    cout << "============================================================" << endl;
    cout << "  内存模型与RAII - 中级进阶Demo" << endl;
    cout << "============================================================" << endl;

    // 一、内存分区演示
    cout << "\n=== 1. C++内存分区演示 ===" << endl;
    cout << "  全局变量 g_globalVar 地址: " << &g_globalVar
         << " 值: " << g_globalVar << " → 全局/静态区" << endl;
    cout << "  未初始化全局 g_uninitGlobal 地址: " << &g_uninitGlobal
         << " 值: " << g_uninitGlobal << " → BSS段(零初始化)" << endl;
    cout << "  静态全局 s_staticVar 地址: " << &s_staticVar
         << " 值: " << s_staticVar << " → 全局/静态区" << endl;
    cout << "  全局常量 G_CONST 地址: " << (void*)&G_CONST
         << " 值: " << G_CONST << " → 常量区（只读）" << endl;
    cout << "  字符串常量 \"hello\" 地址: " << (void*)"hello"
         << " → 常量区" << endl;
    cout << "  函数 main 的地址: " << (void*)main
         << " → 代码区" << endl;

    // 二、栈内存
    stackDemo_basic();
    stackDemo_callChain(5);

    // 三、堆内存
    heapDemo_newDelete();
    heapDemo_mallocFree();
    heapDemo_fragmentation();

    // 四、栈vs堆
    stackVsHeap();

    // 五、常见内存错误
    memoryErrors_demo();

    // 六、RAII
    raiiDemo_intArray();
    raiiDemo_fileHandle();
    raiiDemo_lock();

    // 七、智能指针初体验
    smartPtr_intro();

    // 八、综合对比
    noRaiiExample();
    withRaiiExample();

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "  徒弟啊，今天讲的都是干货，你得反复琢磨。" << endl;
    cout << endl;
    cout << "  1. 不懂内存，就不懂C++。C++给了你直接操作内存的能力，" << endl;
    cout << "     这是力量，也是责任。你得对每一块内存负责。" << endl;
    cout << endl;
    cout << "  2. 栈内存快但小，堆内存大但慢。该用哪个心里要有数。" << endl;
    cout << "     小对象、生命周期短的，直接放栈上。" << endl;
    cout << endl;
    cout << "  3. RAII是C++最伟大的思想，没有之一。" << endl;
    cout << "     资源获取即初始化——构造函数拿资源，析构函数还资源。" << endl;
    cout << "     用栈对象管理堆资源，这就是C++的精髓。" << endl;
    cout << "     不仅内存，文件、锁、套接字、连接...凡是需要手动释放的，" << endl;
    cout << "     都应该用RAII包一层。" << endl;
    cout << endl;
    cout << "  4. 能不用裸指针就不用裸指针。优先用智能指针，" << endl;
    cout << "     优先用 unique_ptr，需要共享才用 shared_ptr。" << endl;
    cout << endl;
    cout << "  5. 写代码的时候，脑子里要有一张内存图。" << endl;
    cout << "     每个变量在哪，什么时候生，什么时候死，" << endl;
    cout << "     谁拥有谁，谁管理谁——都得清楚。" << endl;
    cout << "============================================================" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习（回去自己做）" << endl;
    cout << "============================================================" << endl;
    cout << "  1. 写一个 RAII 类管理 socket 连接（伪代码也行）" << endl;
    cout << "  2. 验证一下：在函数里new一个对象，然后return之前抛异常，" << endl;
    cout << "     看看不用智能指针和用智能指针的区别（用try-catch包裹）" << endl;
    cout << "  3. 用 unique_ptr 实现一个简单的动态数组类" << endl;
    cout << "  4. 思考：为什么栈比堆快？从操作系统角度分析" << endl;
    cout << "  5. 动手画一张C++内存分区图，标清楚每块存什么" << endl;
    cout << "============================================================" << endl;

    return 0;
}
