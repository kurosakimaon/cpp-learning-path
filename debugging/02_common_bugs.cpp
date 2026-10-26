/*
 * ============================================================
 *   C++ 常见 bug 大全（含详细分析）
 *   老师傅说：知道坑在哪，才能避开坑
 * ============================================================
 *
 *  学完这个文件，你要认识这些常见 bug：
 *    1. 内存类 bug：越界、泄漏、野指针、重复释放
 *    2. 字符串类 bug：\0 问题、string 引用失效
 *    3. 类与对象 bug：深浅拷贝、虚函数误用
 *    4. 隐式转换 bug：整型提升、隐式构造
 *    5. 其他经典 bug：整数溢出、有符号/无符号混用
 *
 *  每个 bug 都有：有问题的代码 + 问题分析 + 修复后的代码
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <memory>
#include <limits>

using std::cout;
using std::endl;
using std::string;

// ============================================================
//  Bug 1：数组越界（最常见的 bug 之一）
// ============================================================
//
//  症状：程序崩溃、莫名其妙的值、随机崩溃
//  原因：C/C++ 不检查数组下标是否越界，越界了直接读写别的内存
//  后果：轻则数据错，重则崩溃，甚至安全漏洞

void bug_array_out_of_bounds() {
    cout << "\n=== Bug 1: 数组越界 ===" << endl;

    int arr[5] = {1, 2, 3, 4, 5};

    // ❌ 错误：数组大小是 5，下标只能到 4
    // 读 arr[5] 属于未定义行为：可能读到垃圾值、崩溃，或"看起来正常"。
    // 这里不真正执行越界访问，避免演示程序自己踩 UB。
    cout << "arr[5] 是越界访问（未定义行为），这里只讲解，不真正读取。" << endl;

    // ❌ 更危险：越界写，会破坏别的变量或栈帧
    // arr[100] = 999;  // 取消注释可能导致程序崩溃

    // ✅ 正确：下标范围 0 ~ size-1
    for (int i = 0; i < 5; i++) {
        cout << "arr[" << i << "] = " << arr[i] << endl;
    }

    cout << "💡 记住：数组大小是 N，下标从 0 到 N-1" << endl;
}

// ============================================================
//  Bug 2：野指针（指向已经释放的内存）
// ============================================================
//
//  症状：有时正常有时崩溃（最讨厌的那种 bug）
//  原因：delete 之后指针还在用，或者返回了局部变量的地址
//  后果：内存已经被别人用了，你读出来的是垃圾，写出去会破坏别人数据

void bug_dangling_pointer() {
    cout << "\n=== Bug 2: 野指针 ===" << endl;

    int* p = new int(42);
    cout << "delete 之前: *p = " << *p << endl;

    delete p;
    // ❌ 错误：delete 之后 p 变成悬空指针（野指针），不能再解引用。
    // 解引用已释放的内存是未定义行为，所以这里不演示 *p，只说明后果。
    cout << "delete 之后，p 已成为悬空指针，不能解引用。" << endl;

    // ✅ 好习惯：delete 之后立刻把指针设为 nullptr
    p = nullptr;
    // 现在再用 p 就会直接崩溃（总比随机错好）
    // cout << *p << endl;  // 取消注释试试，会直接段错误

    cout << "💡 好习惯：delete 后立刻设为 nullptr" << endl;
    cout << "💡 更好的习惯：用智能指针，不用手动 delete" << endl;
}

// ============================================================
//  Bug 3：内存泄漏（分配了不释放）
// ============================================================
//
//  症状：程序越跑越慢，最后崩了
//  原因：new 了之后忘记 delete，或者异常导致 delete 没执行到
//  后果：内存越用越少，最终 OOM（Out of Memory）

void bug_memory_leak() {
    cout << "\n=== Bug 3: 内存泄漏 ===" << endl;

    // 模拟泄漏场景
    auto leakyFunction = []() {
        int* p = new int[1000];  // 分配了
        // ... 一些操作 ...
        // 假设这里抛出了异常，下面的 delete 就执行不到了
        // 或者干脆就是忘了写 delete
        // delete[] p;  // ← 被注释掉了，内存泄漏！
    };

    leakyFunction();
    leakyFunction();
    leakyFunction();

    cout << "调用了 3 次 leakyFunction，每次泄漏 1000 个 int 的内存" << endl;
    cout << "一次两次看不出，跑一天就崩了" << endl;

    // ✅ 正确：用 RAII（智能指针），自动释放
    auto safeFunction = []() {
        auto p = std::make_unique<int[]>(1000);
        // 不管函数怎么退出（正常返回还是异常），都会自动释放
    };

    safeFunction();  // 不会泄漏！

    cout << "💡 用智能指针，从根源上消除内存泄漏" << endl;
    cout << "💡 检查工具：Linux 下用 valgrind，或者 AddressSanitizer" << endl;
}

// ============================================================
//  Bug 4：深浅拷贝（最经典的 C++ bug）
// ============================================================
//
//  症状：两个对象同时释放同一块内存，double free 崩溃
//  原因：默认拷贝构造函数是浅拷贝，只复制指针的值，不复制指针指向的内容

class ShallowString {
    char* data_;
public:
    ShallowString(const char* s) {
        data_ = new char[strlen(s) + 1];
        strcpy(data_, s);
    }
    // ❌ 没有自定义拷贝构造函数，编译器生成的是浅拷贝
    // 只复制 data_ 指针的值，两个对象指向同一块内存

    ~ShallowString() {
        delete[] data_;  // 两个对象析构时都 delete 同一块内存 → 崩溃！
    }
    const char* c_str() const { return data_; }
};

// ✅ 正确的深拷贝版本
class DeepString {
    char* data_;
public:
    DeepString(const char* s) {
        data_ = new char[strlen(s) + 1];
        strcpy(data_, s);
    }

    // ✅ 自定义拷贝构造：深拷贝
    DeepString(const DeepString& other) {
        data_ = new char[strlen(other.data_) + 1];
        strcpy(data_, other.data_);
    }

    // ✅ 自定义赋值运算符：深拷贝（注意自赋值检查）
    DeepString& operator=(const DeepString& other) {
        if (this != &other) {        // 防止自赋值
            // 先分配并复制新内容，成功后再释放旧内存。
            // 若先 delete[] 再 new，new 抛异常时对象会留下悬空指针，
            // 析构时还会再次 delete[]（双重释放）。
            char* fresh = new char[strlen(other.data_) + 1];
            strcpy(fresh, other.data_);
            delete[] data_;
            data_ = fresh;
        }
        return *this;
    }

    ~DeepString() {
        delete[] data_;
    }
    const char* c_str() const { return data_; }
};

void bug_shallow_copy() {
    cout << "\n=== Bug 4: 浅拷贝 vs 深拷贝 ===" << endl;

    DeepString s1("hello");
    DeepString s2 = s1;  // 深拷贝，各自有一份数据

    cout << "深拷贝后：s1 = " << s1.c_str() << ", s2 = " << s2.c_str() << endl;
    cout << "两个对象各自有独立的内存，析构时互不影响" << endl;

    // ❌ 如果用 ShallowString，下面的代码运行到函数结束就会崩溃
    // ShallowString s1("hello");
    // ShallowString s2 = s1;  // 浅拷贝，两个指针指向同一块内存
    // 函数结束时，s2 先析构（释放内存），s1 再析构（重复释放 → 崩溃）

    cout << "💡 记住三大定律：三法则、五法则、零法则" << endl;
    cout << "  三法则：自定义了析构/拷贝构造/赋值中的一个，就要写另外两个" << endl;
    cout << "  五法则：C++11 加上移动构造和移动赋值" << endl;
    cout << "  零法则：尽量用智能指针和标准容器，让编译器帮你管理" << endl;
}

// ============================================================
//  Bug 5：整数溢出
// ============================================================
//
//  症状：算出来的数莫名其妙变成负数，或者很小
//  原因：整型的范围是有限的
//        有符号整数溢出是未定义行为（UB），不要依赖它“环绕”
//        无符号整数溢出是有定义的：按 2^N 取模环绕

void bug_integer_overflow() {
    cout << "\n=== Bug 5: 整数溢出 ===" << endl;

    // int 的范围大约是 -21亿 ~ +21亿
    int a = 2000000000;
    int b = 2000000000;

    // ❌ int sum = a + b;  // 有符号溢出是未定义行为，结果不可预测！
    // 正确做法：先判断再计算，或者提前换成更宽的类型
    if (a > std::numeric_limits<int>::max() - b) {
        cout << "a + b 会超出 int 范围，已阻止这次计算" << endl;
    }

    // ✅ 正确：用更大的类型
    long long bigSum = (long long)a + b;
    cout << "用 long long: a + b = " << bigSum << "  ← 正确" << endl;

    // 另一个经典坑：无符号数相减
    // 无符号运算不会得到负数，而是按 2^32 取模，变成很大的正数
    unsigned int x = 5;
    unsigned int y = 10;
    unsigned int diff = x - y;  // 有定义，但通常不是想要的结果
    cout << "\n5u - 10u = " << diff << "  ← 无符号环绕，变成超大数！" << endl;

    cout << "💡 关键检查点：两数相加/相乘时，想想会不会超范围" << endl;
    cout << "💡 有符号溢出是 UB；无符号溢出按模 2^N 环绕，二者都不是“正常结果”" << endl;
}

// ============================================================
//  Bug 6：有符号 / 无符号比较
// ============================================================
//
//  症状：大小比较结果完全不对
//  原因：有符号数和无符号数比较时，有符号数会被转成无符号数
//        负数会变成超大的正数

void bug_signed_unsigned() {
    cout << "\n=== Bug 6: 有符号/无符号混用 ===" << endl;

    int a = -1;
    unsigned int b = 1;

    if (a < b) {
        cout << "-1 < 1，正常" << endl;
    } else {
        cout << "❌ -1 >= 1？不可能吧？但这就是结果！" << endl;
        cout << "   原因：-1 被转成无符号数，变成了 4294967295，当然比 1 大" << endl;
    }

    // 另一个常见场景：vector 的 size() 是无符号的
    std::vector<int> v{1, 2, 3};
    int n = -1;
    bool greater = v.size() > n;   // n 被转换成 unsigned，-1 变成超大正数
    cout << "v.size()(" << v.size() << ") > -1 的结果是: "
         << (greater ? "true" : "false") << endl;
    if (!greater) {
        cout << "❌ 答案是 false！因为 -1 被转成无符号数，变成超大的正数" << endl;
    }

    cout << "💡 尽量不要混用有符号和无符号" << endl;
    cout << "💡 如果一定要比，先明确转换（确保值是正的）" << endl;
}

// ============================================================
//  Bug 7：返回局部变量的指针/引用
// ============================================================
//
//  症状：返回的值是乱码、随机崩溃
//  原因：局部变量在栈上，函数返回后栈帧被回收，那块内存随时会被覆盖

// ❌ 错误：返回了局部变量的指针
int* bug_return_local_ptr() {
    int x = 42;
    return &x;  // x 是局部变量，函数返回后就没了！
}

// ❌ 错误：返回了局部变量的引用
int& bug_return_local_ref() {
    int x = 42;
    return x;  // 同样的问题
}

void bug_return_local() {
    cout << "\n=== Bug 7: 返回局部变量的指针/引用 ===" << endl;

    int* p = bug_return_local_ptr();
    // cout << "*p = " << *p << endl;  // 可能刚好是 42（还没被覆盖）
    // 调用几个函数之后，再来看就变成垃圾了
    cout << "返回局部变量的指针 = 野指针，迟早出问题" << endl;

    cout << "💡 局部变量存在栈上，函数返回就没了" << endl;
    cout << "💡 要返回对象就直接返回（值返回），编译器会做 RVO 优化" << endl;
    cout << "💡 大对象用动态分配（返回智能指针）" << endl;
}

// ============================================================
//  Bug 8：隐式类型转换导致的意外
// ============================================================
//
//  症状：传进去的参数不对，但编译器不报错
//  原因：单参数构造函数支持隐式转换，或者内置类型隐式转换

class User {
    int age_;
public:
    // ❌ 单参数构造函数可以隐式转换
    // 传一个 int 进去会自动构造成 User 对象
    User(int age) : age_(age) {}

    int age() const { return age_; }
};

void printUser(const User& user) {
    cout << "用户年龄: " << user.age() << endl;
}

void bug_implicit_conversion() {
    cout << "\n=== Bug 8: 隐式类型转换 ===" << endl;

    printUser(25);  // ❌ 居然能编译通过！
    // 25 被隐式转换成了 User(25)
    // 如果是不小心传错了参数，编译器不会报错，bug 就藏下来了

    // ✅ 正确：构造函数加 explicit 关键字，禁止隐式转换
    // explicit User(int age) : age_(age) {}
    // 加上之后 printUser(25) 就编译不过了

    cout << "printUser(25) 居然编译通过了？" << endl;
    cout << "因为 int 隐式转换成了 User 对象" << endl;
    cout << "💡 单参数构造函数默认加 explicit，除非你确实想要隐式转换" << endl;
}

// ============================================================
//  Bug 9：for 循环中的迭代器失效
// ============================================================
//
//  症状：vector 遍历时插入删除，程序崩溃或结果不对
//  原因：vector 的插入/删除可能导致迭代器失效

void bug_iterator_invalidation() {
    cout << "\n=== Bug 9: 迭代器失效 ===" << endl;

    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    // ❌ 错误：遍历时删除元素，迭代器失效了
    // for (auto it = v.begin(); it != v.end(); ++it) {
    //     if (*it % 2 == 0) {
    //         v.erase(it);  // erase 之后 it 失效了，再 ++it 就崩了
    //     }
    // }

    // ✅ 正确方法1：利用 erase 的返回值
    auto it = v.begin();
    while (it != v.end()) {
        if (*it % 2 == 0) {
            it = v.erase(it);  // erase 返回下一个有效的迭代器
        } else {
            ++it;
        }
    }

    cout << "删除偶数后: ";
    for (int x : v) cout << x << " ";
    cout << endl;

    // ✅ 更好的方法：erase-remove 惯用法（C++20 之前）
    std::vector<int> v2 = {1, 2, 3, 4, 5, 6};
    v2.erase(std::remove(v2.begin(), v2.end(), 3), v2.end());
    cout << "erase-remove 删除 3 后: ";
    for (int x : v2) cout << x << " ";
    cout << endl;

    cout << "💡 vector 插入/删除元素后，迭代器可能失效" << endl;
    cout << "💡 用 erase 的返回值，或者用 erase-remove 惯用法" << endl;
    cout << "💡 C++20 有 std::erase/std::erase_if，更简单" << endl;
}

// ============================================================
//  Bug 10：= 和 == 搞混
// ============================================================
//
//  症状：if 判断永远成立（或永远不成立）
//  原因：把比较运算符 == 写成了赋值运算符 =

void bug_assignment_in_if() {
    cout << "\n=== Bug 10: = 和 == 搞混 ===" << endl;

    int x = 5;

    // ❌ 错误：= 是赋值，不是比较！
    if (x = 10) {  // 这行把 x 赋值为 10，然后判断 10 是否为真（永远为真）
        cout << "x 等于 10" << endl;
    }

    // ✅ 正确：用 ==
    x = 5;
    if (x == 10) {
        cout << "x 等于 10" << endl;
    } else {
        cout << "x 不等于 10" << endl;
    }

    cout << "💡 一个小技巧：把常量写左边  if (10 == x)" << endl;
    cout << "   这样万一写成 = 号，编译器会报错（10 = x 不合法）" << endl;
    cout << "💡 开警告：-Wall -Wextra 会警告这种情况" << endl;
}

// ============================================================
//  老师傅的话
// ============================================================
//
//  徒弟，看完这 10 个常见 bug，你可能会觉得：
//  "C++ 怎么这么多坑？"
//
//  师傅说：坑多是因为 C++ 给你太多自由了。
//  自由不是坏事，但自由需要能力来驾驭。
//
//  怎么少踩坑？
//
//  1. 开警告！开警告！开警告！
//     -Wall -Wextra -Wpedantic，把警告当错误看（-Werror）
//     大部分 bug 编译器都能帮你找出来
//
//  2. 用工具！
//     - AddressSanitizer：内存问题一抓一个准
//     - Valgrind：内存泄漏检测
//     - clang-tidy：静态分析
//
//  3. 用现代 C++ 特性
//     - 用智能指针代替裸指针
//     - 用标准容器代替裸数组
//     - 用 string 代替 char*
//     能少写多少 bug 你知道吗？
//
//  4. Code Review
//     自己的 bug 自己最难发现，别人一眼就看出来了
//
//  5. 写单元测试
//     你以为写对了？跑个测试试试
//
//  小练习：
//  1. 每个 bug 都自己写一下，看看实际运行结果是不是和你想的一样
//  2. 想一个你之前写代码时遇到的 bug，把它归到上面哪一类
//  3. 试试用 -Wall -Wextra 编译，看看有多少警告

int main() {
    cout << "========== C++ 常见 Bug 大全 ==========" << endl;

    bug_array_out_of_bounds();
    bug_dangling_pointer();
    bug_memory_leak();
    bug_shallow_copy();
    bug_integer_overflow();
    bug_signed_unsigned();
    bug_return_local();
    bug_implicit_conversion();
    bug_iterator_invalidation();
    bug_assignment_in_if();

    cout << "\n========== 总结 ==========" << endl;
    cout << "10 个常见 bug，你中过几个？" << endl;
    cout << "踩过坑的印象才深刻。没踩过的，小心点，迟早会遇到。" << endl;
    cout << "下一课：用工具找 bug（Sanitizers 和调试器）" << endl;

    return 0;
}
