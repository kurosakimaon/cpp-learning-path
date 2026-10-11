/*
 * ============================================================
 *   Sanitizers 与内存调试
 *   老师傅说：AddressSanitizer 是 C++ 程序员的神器
 * ============================================================
 *
 *  学完这个文件，你要了解：
 *    1. AddressSanitizer (ASAN) —— 内存问题检测器
 *    2. UndefinedBehaviorSanitizer (UBSAN) —— 未定义行为检测器
 *    3. MemorySanitizer (MSAN) —— 未初始化内存读取
 *    4. ThreadSanitizer (TSAN) —— 数据竞争检测
 *
 *  编译方法（GCC/Clang）：
 *    ASAN:  g++ -fsanitize=address -g 03_sanitizers.cpp -o demo_asan
 *    UBSAN: g++ -fsanitize=undefined -g 03_sanitizers.cpp -o demo_ubsan
 *    TSAN:  g++ -fsanitize=thread -g 03_sanitizers.cpp -o demo_tsan
 *
 *  MSVC 用户：项目属性 → C/C++ → 常规 → 启用地址擦除器
 *
 *  注意：这些 bug 用普通编译跑可能没事，加了 sanitizer 才会被检测出来
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>

using std::cout;
using std::endl;

// ============================================================
//  ASAN 能检测的问题 1：堆缓冲区溢出（Heap Buffer Overflow）
// ============================================================
//
//  描述：在堆上分配的数组，访问越界了
//  普通编译：可能不崩（运气好越界的内存没用上）
//  ASAN：立刻报错，精确告诉你越界了多少字节

void asan_heap_overflow() {
    cout << "\n=== ASAN: 堆缓冲区溢出 ===" << endl;

    int* arr = new int[10];  // 分配 10 个 int
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }

    // ❌ 越界写：写到了第 10 个元素后面
    // 普通编译可能没事（因为 malloc 通常多分配一点）
    // ASAN 会直接报错：heap-buffer-overflow
    cout << "尝试越界写 arr[10]..." << endl;
    // arr[10] = 999;  // 取消注释，用 ASAN 编译运行就会报错

    cout << "（已注释掉，取消注释后用 -fsanitize=address 编译运行试试）" << endl;

    delete[] arr;
}

// ============================================================
//  ASAN 能检测的问题 2：栈缓冲区溢出（Stack Buffer Overflow）
// ============================================================

void asan_stack_overflow() {
    cout << "\n=== ASAN: 栈缓冲区溢出 ===" << endl;

    char buf[8];  // 栈上分配 8 字节

    // ❌ 栈越界：写了 10 个字节
    // strcpy(buf, "hello world");  // 取消注释试试
    // ASAN 报错：stack-buffer-overflow

    cout << "栈溢出比堆溢出更危险，可能被利用执行恶意代码" << endl;
    cout << "（已注释，取消注释后用 ASAN 编译试试）" << endl;
}

// ============================================================
//  ASAN 能检测的问题 3：Use After Free（释放后使用）
// ============================================================
//
//  描述：delete 之后还在用那块内存
//  普通编译：可能刚好没被覆盖，看起来正常
//  ASAN：立刻报错

void asan_use_after_free() {
    cout << "\n=== ASAN: Use After Free（释放后使用） ===" << endl;

    int* p = new int(42);
    cout << "delete 前: *p = " << *p << endl;

    delete p;

    // ❌ 释放后还在用
    // cout << "delete 后: *p = " << *p << endl;
    // ASAN 报错：heap-use-after-free

    cout << "野指针问题，ASAN 一抓一个准" << endl;
    cout << "（已注释，取消注释后用 ASAN 编译试试）" << endl;
}

// ============================================================
//  ASAN 能检测的问题 4：Double Free（重复释放）
// ============================================================

void asan_double_free() {
    cout << "\n=== ASAN: Double Free（重复释放） ===" << endl;

    int* p = new int(42);

    delete p;
    // delete p;  // ❌ 又释放一次！
    // ASAN 报错：heap-double-free

    cout << "重复释放，普通编译可能崩溃，也可能不崩（看内存分配器）" << endl;
    cout << "用 ASAN 一定报错" << endl;
    cout << "（已注释，取消注释后用 ASAN 编译试试）" << endl;
}

// ============================================================
//  ASAN 能检测的问题 5：内存泄漏（Memory Leak）
// ============================================================

void asan_memory_leak() {
    cout << "\n=== ASAN: 内存泄漏检测 ===" << endl;

    // ❌ 分配了不释放
    // int* p = new int[100];
    // 函数返回后 p 没了，但内存没释放 → 泄漏

    cout << "ASAN 退出时会统计泄漏的内存大小和调用栈" << endl;
    cout << "用 ASAN_OPTIONS=detect_leaks=1 开启泄漏检测" << endl;
    cout << "（已注释掉泄漏代码，你可以自己加几行试试）" << endl;
}

// ============================================================
//  UBSAN 能检测的问题：未定义行为
// ============================================================
//
//  C++ 里有很多"未定义行为"（Undefined Behavior, UB）
//  意思是：标准没规定这种情况该怎么办，编译器想怎么来都行
//  UBSAN 能帮你找出这些未定义行为

void ubsan_demo() {
    cout << "\n=== UBSAN: 未定义行为检测 ===" << endl;

    // 1. 整数溢出（有符号数）
    int a = 2147483647;  // int 最大值
    // int b = a + 1;    // ❌ 有符号整数溢出是 UB！
    // cout << "a + 1 = " << b << endl;

    // 2. 除零
    int x = 10;
    int y = 0;
    // int z = x / y;   // ❌ 除零是 UB

    // 3. 移位溢出
    int n = 1;
    // int m = n << 100;  // ❌ 移位数超过位数是 UB

    // 4. 空指针解引用
    int* p = nullptr;
    // *p = 42;  // ❌ 解引用空指针

    // 5. 对齐错误
    // （比较复杂，一般很少遇到）

    cout << "UBSAN 能检测的未定义行为：" << endl;
    cout << "  - 有符号整数溢出" << endl;
    cout << "  - 除零" << endl;
    cout << "  - 移位溢出" << endl;
    cout << "  - 空指针解引用" << endl;
    cout << "  - 对齐错误" << endl;
    cout << "  - 等等..." << endl;
    cout << "（上面的例子都注释掉了，取消注释用 UBSAN 编译试试）" << endl;
}

// ============================================================
//  TSAN 能检测的问题：数据竞争
// ============================================================
//
//  多线程下，两个线程同时访问同一个变量，至少有一个在写，
//  而且没有同步 → 数据竞争 → 未定义行为
//
//  这是并发编程里最隐蔽的 bug 之一，TSAN 是最好的检测工具

void tsan_demo() {
    cout << "\n=== TSAN: 数据竞争检测 ===" << endl;

    int counter = 0;
    const int N = 100000;

    // ❌ 两个线程都在写 counter，没有锁 → 数据竞争
    auto worker = [&]() {
        for (int i = 0; i < N; i++) {
            counter++;  // 数据竞争！
        }
    };

    // std::thread t1(worker);
    // std::thread t2(worker);
    // t1.join();
    // t2.join();
    // cout << "counter = " << counter << " (应该是 " << 2*N << ")" << endl;
    // TSAN 会报错：data race

    cout << "数据竞争：两个线程同时写同一个变量" << endl;
    cout << "普通编译：结果不对，偶现崩溃" << endl;
    cout << "TSAN：精确告诉你哪个变量、哪两行代码在竞争" << endl;
    cout << "（已注释，取消注释后用 -fsanitize=thread 编译试试）" << endl;
}

// ============================================================
//  调试辅助技巧：打印调用栈
// ============================================================
//
//  程序崩了，你想知道是从哪个路径调用过来的
//  这时候打印调用栈（call stack）非常有用

#include <execinfo.h>
#include <cstdlib>

void printCallStack() {
    cout << "\n=== 打印调用栈 ===" << endl;
    cout << "（这个功能需要 Linux/macOS，Windows 下用调试器看）" << endl;

    // 注意：下面这段代码在 Windows 上编译不过，所以注释掉
    // Linux/macOS 下取消注释试试
    /*
    void* array[20];
    int size = backtrace(array, 20);
    char** strings = backtrace_symbols(array, size);
    cout << "调用栈（共 " << size << " 帧）：" << endl;
    for (int i = 0; i < size; i++) {
        cout << "  [" << i << "] " << strings[i] << endl;
    }
    free(strings);
    */

    cout << "调用栈可以告诉你：程序是怎么一步步走到这里来的" << endl;
    cout << "排查 bug 的时候超级有用！" << endl;
}

// ============================================================
//  老师傅的话
// ============================================================
//
//  徒弟，这些 Sanitizer 工具，师傅强烈建议你：
//
//  1. 开发阶段默认开 ASAN + UBSAN
//     加个编译选项而已，但能帮你找出 80% 的内存 bug
//     虽然会慢一点（大概 2 倍），但开发阶段完全可以接受
//
//  2. 写多线程代码，开 TSAN 跑一遍
//     数据竞争这种 bug，靠眼睛找不出来的
//     TSAN 一跑，清清楚楚
//
//  3. 单元测试一定要开 Sanitizer 跑
//     测试的目的就是找 bug，有工具不用白不用
//
//  4. 不要忽略任何警告和错误
//     Sanitizer 报了错就一定要修，不要侥幸
//     今天不报不代表明天不报，线上崩了更惨
//
//  常用编译选项：
//    Debug + Sanitizer:
//      -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
//
//    Thread Sanitizer:
//      -g -O1 -fsanitize=thread
//
//    为什么 -O1？
//      -O0 太慢，-O2 又优化太多信息不好找，-O1 是平衡点
//
//    为什么 -fno-omit-frame-pointer？
//      保留帧指针，调用栈更准确
//
//  小练习：
//  1. 取消上面例子中的注释，用 ASAN 编译运行，看看报错信息长什么样
//  2. 自己写一个有内存 bug 的程序，用 ASAN 能不能找出来
//  3. 试试 UBSAN，故意写一个整数溢出看看

int main() {
    cout << "========== Sanitizers 与内存调试 ==========" << endl;
    cout << "提示：这些 demo 需要用 -fsanitize 选项编译才能看到效果" << endl;
    cout << "普通编译下，很多 bug 是"隐身"的" << endl;

    asan_heap_overflow();
    asan_stack_overflow();
    asan_use_after_free();
    asan_double_free();
    asan_memory_leak();

    ubsan_demo();
    tsan_demo();
    printCallStack();

    cout << "\n========== 总结 ==========" << endl;
    cout << "Sanitizer 家族：" << endl;
    cout << "  ASAN  - 内存问题（越界、UAF、泄漏等）⭐⭐⭐⭐⭐" << endl;
    cout << "  UBSAN - 未定义行为（溢出、除零等）⭐⭐⭐⭐" << endl;
    cout << "  TSAN  - 数据竞争（多线程）⭐⭐⭐⭐" << endl;
    cout << "  MSAN  - 未初始化读取（Clang 独有）⭐⭐⭐" << endl;
    cout << endl;
    cout << "开 Sanitizer 开发，你会回来感谢我的。" << endl;

    return 0;
}
