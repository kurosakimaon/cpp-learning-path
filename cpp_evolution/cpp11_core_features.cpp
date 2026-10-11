/**
 * @file cpp11_core_features.cpp
 * @brief C++11 核心特性深度解析（现代C++的基石）
 *
 * C++11 是 C++ 历史上最重要的版本之一，被誉为 "现代 C++ 的起点"。
 * 本文件深入讲解 C++11 引入的核心特性，以及它们如何改变了 C++ 的编程范式。
 *
 * 主要特性概览：
 * 1. 类型推导：auto、decltype、尾置返回类型
 * 2. 右值引用与移动语义：移动构造/赋值、std::move、完美转发
 * 3. Lambda 表达式：语法、捕获、闭包类型
 * 4. 智能指针：unique_ptr、shared_ptr、weak_ptr
 * 5. nullptr 与类型安全
 * 6. 范围 for 循环
 * 7. 初始化列表与统一初始化
 * 8. constexpr (C++11 版)
 * 9. 变参模板 (Variadic Templates)
 * 10. 别名模板 (Template Aliases)
 * 11. 线程支持库：std::thread、std::mutex、std::condition_variable
 * 12. 新容器：std::array、std::forward_list、unordered 系列
 * 13. default / delete 函数
 * 14. override / final 说明符
 * 15. std::tuple
 * 16. 强类型枚举 (enum class)
 *
 * 编译命令: g++ -std=c++11 -Wall -Wextra -pedantic cpp11_core_features.cpp -o cpp11_core -pthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <array>
#include <unordered_map>
#include <tuple>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <initializer_list>
#include <utility>
#include <sstream>

// ============================================================================
// 1. auto 与 decltype —— 类型推导
// ============================================================================
namespace section1_auto_decltype {

// auto 的本质：让编译器从初始化表达式推导变量类型
// decltype 的本质：查询表达式的类型（包括引用和顶层const）
// 两者的区别：auto 会忽略引用和顶层const，decltype 保留

int foo() { return 42; }
int& bar() { static int x = 10; return x; }
const int& baz() { static int x = 20; return x; }

// 尾置返回类型（C++11 引入，配合 decltype 实现返回类型推导）
// 经典应用：返回类型依赖于参数类型
template <typename T, typename U>
auto add(const T& t, const U& u) -> decltype(t + u) {
    return t + u;
}

// decltype 的两种用法：
// decltype(entity)  -> 返回实体的声明类型
// decltype(expr)    -> 返回表达式的值类别对应的类型：
//   - 左值表达式  -> T&
//   - 纯右值      -> T
//   - 将亡值      -> T&&
void demo() {
    std::cout << "\n===== 1. auto 与 decltype =====\n";

    // auto 推导（忽略引用和顶层const）
    auto a1 = foo();     // int
    auto a2 = bar();     // int（引用被去掉）
    auto a3 = baz();     // int（const和引用都被去掉）

    // decltype 保留完整类型
    decltype(foo()) d1 = 0;       // int
    decltype(bar()) d2 = bar();   // int&
    decltype(baz()) d3 = baz();   // const int&

    std::cout << "auto 推导结果都是值类型，decltype 保留引用和const\n";

    // 尾置返回类型的应用
    auto result = add(std::string("hello "), std::string("world"));
    std::cout << "add(string, string) = " << result << "\n";

    auto num_result = add(1, 2.5);  // double
    std::cout << "add(int, double) = " << num_result << "\n";

    // decltype 的进阶：左值表达式 vs 右值表达式
    int x = 10;
    int* p = &x;

    // 表达式的值类别决定 decltype 的结果：
    // - 如果 expr 是 prvalue（纯右值），结果是 T
    // - 如果 expr 是 lvalue（左值），结果是 T&
    // - 如果 expr 是 xvalue（将亡值），结果是 T&&
    decltype(x)     var1 = x;   // int（x是id表达式，属于entity用法）
    decltype((x))   var2 = x;   // int&（(x)是左值表达式）
    decltype(*p)    var3 = x;   // int&（*p是左值表达式）
    decltype(x++)   var4 = 10;  // int（x++是纯右值）
    decltype(std::move(x)) var5 = 10; // int&&（std::move(x)是将亡值）

    std::cout << "decltype((x)) 是 int&，因为 (x) 是左值表达式\n";
    std::cout << "decltype(*p)  是 int&，因为解引用返回左值\n";
    std::cout << "decltype(x++) 是 int，因为后置++返回纯右值\n";

    // 经典用法：获取函数返回类型
    using FuncType = decltype(&add<int, double>);
    std::cout << "add<int,double> 的类型可以通过 decltype 获取\n";
}

} // namespace section1_auto_decltype

// ============================================================================
// 2. 右值引用与移动语义
// ============================================================================
namespace section2_move_semantics {

// 为什么需要移动语义？
// - 传统拷贝构造会进行深拷贝，对于持有资源的对象开销很大
// - 右值引用允许我们"偷取"临时对象的资源，避免不必要的拷贝
//
// 值类别体系（C++11 后）：
// - 左值 (lvalue): 有身份、不能被移动 -> T& 绑定
// - 将亡值 (xvalue): 有身份、可以被移动 -> T&& 绑定
// - 纯右值 (prvalue): 无身份、可以被移动 -> const T& 或 T&& 绑定
// 其中 xvalue + prvalue 统称为右值 (rvalue)

class BigData {
public:
    BigData(size_t size = 1024) : size_(size), data_(new int[size]) {
        std::cout << "  BigData 构造（分配 " << size_ << " 个元素）\n";
        std::fill(data_, data_ + size_, 0);
    }

    // 拷贝构造函数（深拷贝）
    BigData(const BigData& other) : size_(other.size_), data_(new int[other.size_]) {
        std::cout << "  BigData 拷贝构造（深拷贝 " << size_ << " 个元素）\n";
        std::copy(other.data_, other.data_ + other.size_, data_);
    }

    // 移动构造函数（窃取资源）
    // noexcept 很重要：保证移动不抛异常，STL容器才会优先使用移动
    BigData(BigData&& other) noexcept : size_(other.size_), data_(other.data_) {
        std::cout << "  BigData 移动构造（窃取资源，size=" << size_ << "）\n";
        other.size_ = 0;
        other.data_ = nullptr;  // 关键：将源对象置空
    }

    // 拷贝赋值
    BigData& operator=(const BigData& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + other.size_, data_);
            std::cout << "  BigData 拷贝赋值\n";
        }
        return *this;
    }

    // 移动赋值
    BigData& operator=(BigData&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
            std::cout << "  BigData 移动赋值\n";
        }
        return *this;
    }

    ~BigData() {
        delete[] data_;
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    int* data_;
};

BigData create_big_data() {
    BigData data(2048);
    return data;  // 会触发 NRVO（命名返回值优化）或移动
}

// 完美转发：保持参数的值类别
// 原理：引用折叠 + std::forward
//   T& &   -> T&
//   T& &&  -> T&
//   T&& &  -> T&
//   T&& && -> T&&
template <typename T>
BigData relay(T&& arg) {
    // std::forward<T>(arg) 的效果：
    // - 如果 T 是左值引用，arg 以左值形式转发
    // - 如果 T 不是引用，arg 以右值形式转发
    return BigData(std::forward<T>(arg));
}

void demo() {
    std::cout << "\n===== 2. 右值引用与移动语义 =====\n";

    std::cout << "--- 2.1 移动构造 vs 拷贝构造 ---\n";
    BigData a(1024);
    BigData b = a;           // 拷贝构造（a是左值）
    BigData c = std::move(a); // 移动构造（std::move将左值转为右值）
    std::cout << "  移动后 a.size() = " << a.size() << "（资源已被窃取）\n";

    std::cout << "\n--- 2.2 从函数返回对象 ---\n";
    BigData d = create_big_data();  // 通常被NRVO优化，直接构造在返回值上
    std::cout << "  d.size() = " << d.size() << "\n";

    std::cout << "\n--- 2.3 完美转发 ---\n";
    BigData e(512);
    std::cout << "转发左值：\n";
    BigData f = relay(e);           // 左值 -> 拷贝构造
    std::cout << "转发右值：\n";
    BigData g = relay(std::move(e)); // 右值 -> 移动构造

    std::cout << "\n--- 2.4 vector 中的移动 ---\n";
    std::vector<BigData> vec;
    vec.reserve(3);  // 预分配空间，避免扩容
    std::cout << "push_back 左值：\n";
    BigData h(256);
    vec.push_back(h);           // 拷贝
    std::cout << "push_back 右值：\n";
    vec.push_back(std::move(h)); // 移动
    std::cout << "emplace_back：\n";
    vec.emplace_back(128);      // 直接构造（连移动都省了）
}

} // namespace section2_move_semantics

// ============================================================================
// 3. Lambda 表达式
// ============================================================================
namespace section3_lambda {

// Lambda 语法：[capture](params) mutable -> return_type { body }
//
// 捕获方式：
//   []        - 不捕获
//   [=]       - 值捕获所有用到的变量
//   [&]       - 引用捕获所有用到的变量
//   [x, &y]   - x 值捕获，y 引用捕获
//   [=, &y]   - 默认值捕获，y 引用捕获
//   [&, x]    - 默认引用捕获，x 值捕获
//   [this]    - 捕获 this 指针
//
// 本质：编译器生成一个匿名的闭包类（closure type），重载 operator()
// - 没有捕获的 lambda 可以退化为函数指针
// - mutable 允许修改值捕获的变量

// 高阶函数示例：函数作为参数和返回值
std::function<int(int)> make_multiplier(int factor) {
    // 值捕获 factor
    return [factor](int x) { return x * factor; };
}

// lambda 作为谓词
void demo() {
    std::cout << "\n===== 3. Lambda 表达式 =====\n";

    std::cout << "--- 3.1 基础用法 ---\n";
    auto add = [](int a, int b) { return a + b; };
    std::cout << "lambda add(3, 4) = " << add(3, 4) << "\n";

    // 显示指定返回类型（通常可以自动推导，但复杂情况需要）
    auto divide = [](double a, double b) -> double {
        if (b == 0) return 0;
        return a / b;
    };
    std::cout << "divide(10, 3) = " << divide(10, 3) << "\n";

    std::cout << "\n--- 3.2 捕获方式 ---\n";
    int x = 10, y = 20;

    auto by_value = [=]() { return x + y; };  // 值捕获
    auto by_ref = [&]() { x++; y++; };        // 引用捕获
    auto mixed = [x, &y]() { y += x; };       // 混合捕获

    std::cout << "by_value() = " << by_value() << "\n";
    by_ref();
    std::cout << "by_ref() 后 x=" << x << ", y=" << y << "\n";
    mixed();
    std::cout << "mixed() 后 x=" << x << ", y=" << y << "（x不变，y变）\n";

    // mutable lambda：修改值捕获的变量
    auto counter = [count = 0]() mutable {
        return ++count;
    };
    std::cout << "mutable counter: ";
    for (int i = 0; i < 5; i++) {
        std::cout << counter() << " ";
    }
    std::cout << "\n";

    std::cout << "\n--- 3.3 与 STL 算法结合 ---\n";
    std::vector<int> v = {5, 2, 8, 1, 9, 3, 7, 6, 4};

    // 自定义排序
    std::sort(v.begin(), v.end(), [](int a, int b) {
        return a > b;  // 降序
    });
    std::cout << "降序排序: ";
    for (int n : v) std::cout << n << " ";
    std::cout << "\n";

    // count_if
    int threshold = 5;
    int count = std::count_if(v.begin(), v.end(), [threshold](int n) {
        return n > threshold;
    });
    std::cout << "大于 " << threshold << " 的元素个数: " << count << "\n";

    // accumulate + lambda（自定义操作）
    int product = std::accumulate(v.begin(), v.end(), 1,
        [](int acc, int n) { return acc * n; });
    std::cout << "所有元素的乘积: " << product << "\n";

    std::cout << "\n--- 3.4 闭包作为返回值 ---\n";
    auto times3 = make_multiplier(3);
    auto times5 = make_multiplier(5);
    std::cout << "times3(10) = " << times3(10) << "\n";
    std::cout << "times5(10) = " << times5(10) << "\n";

    // 无捕获的 lambda 可以转换为函数指针
    using FuncPtr = int(*)(int, int);
    FuncPtr fp = [](int a, int b) { return a + b; };
    std::cout << "函数指针调用: " << fp(10, 20) << "\n";
}

} // namespace section3_lambda

// ============================================================================
// 4. 智能指针
// ============================================================================
namespace section4_smart_ptr {

// C++11 引入的三种智能指针：
// - unique_ptr: 独占所有权，不可拷贝，可移动，零开销
// - shared_ptr: 共享所有权，引用计数，开销较大
// - weak_ptr:   不拥有所有权，配合 shared_ptr 解决循环引用
//
// 为什么不使用 auto_ptr？ -> C++11 已废弃，C++17 移除
// auto_ptr 的问题：拷贝语义是移动语义，容易误用

class Resource {
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "  Resource '" << name_ << "' 被创建\n";
    }
    ~Resource() {
        std::cout << "  Resource '" << name_ << "' 被销毁\n";
    }
    void use() const {
        std::cout << "  使用 Resource '" << name_ << "'\n";
    }
    const std::string& name() const { return name_; }
private:
    std::string name_;
};

// unique_ptr 的自定义删除器
struct FileDeleter {
    void operator()(FILE* f) const {
        if (f) {
            std::cout << "  FileDeleter 关闭文件\n";
            fclose(f);
        }
    }
};

// 循环引用问题演示
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // 用 weak_ptr 打破循环
    std::string name;

    Node(const std::string& n) : name(n) {
        std::cout << "  Node '" << name << "' 构造\n";
    }
    ~Node() {
        std::cout << "  Node '" << name << "' 析构\n";
    }
};

void demo() {
    std::cout << "\n===== 4. 智能指针 =====\n";

    std::cout << "--- 4.1 unique_ptr（独占所有权） ---\n";
    {
        std::unique_ptr<Resource> p1 = std::make_unique<Resource>("A");
        p1->use();

        // 不能拷贝
        // std::unique_ptr<Resource> p2 = p1;  // 错误！

        // 可以移动
        std::unique_ptr<Resource> p2 = std::move(p1);
        p2->use();
        std::cout << "  移动后 p1 = " << (p1 ? "非空" : "空") << "\n";

        // 自定义删除器
        std::unique_ptr<FILE, FileDeleter> file(
            fopen("test_tmp.txt", "w"), FileDeleter());
        if (file) {
            fprintf(file.get(), "hello unique_ptr");
        }
    }  // 离开作用域，自动释放

    std::cout << "\n--- 4.2 shared_ptr（共享所有权） ---\n";
    {
        std::shared_ptr<Resource> p1 = std::make_shared<Resource>("B");
        std::cout << "  引用计数: " << p1.use_count() << "\n";
        {
            std::shared_ptr<Resource> p2 = p1;
            std::shared_ptr<Resource> p3 = p2;
            std::cout << "  内部作用域，引用计数: " << p1.use_count() << "\n";
            p2->use();
        }
        std::cout << "  外部作用域，引用计数: " << p1.use_count() << "\n";
    }  // 引用计数归零，自动释放

    std::cout << "\n--- 4.3 weak_ptr 与循环引用 ---\n";
    {
        auto node1 = std::make_shared<Node>("first");
        auto node2 = std::make_shared<Node>("second");

        node1->next = node2;
        node2->prev = node1;  // weak_ptr，不增加引用计数

        std::cout << "  node1 引用计数: " << node1.use_count() << "\n";
        std::cout << "  node2 引用计数: " << node2.use_count() << "\n";

        // weak_ptr 必须 lock() 后才能使用
        if (auto prev = node2->prev.lock()) {
            std::cout << "  通过 weak_ptr 访问: " << prev->name << "\n";
        }
    }  // 如果 prev 是 shared_ptr，这里会内存泄漏

    std::cout << "\n--- 4.4 工厂模式中的应用 ---\n";
    auto factory = []() -> std::unique_ptr<Resource> {
        return std::make_unique<Resource>("C");
    };
    auto r = factory();
    r->use();
    // 可以轻松转为 shared_ptr
    std::shared_ptr<Resource> sp = std::move(r);
    std::cout << "  unique_ptr 转为 shared_ptr 后，r = "
              << (r ? "非空" : "空") << "\n";
}

} // namespace section4_smart_ptr

// ============================================================================
// 5. nullptr、enum class、constexpr 等其他重要特性
// ============================================================================
namespace section5_misc {

// nullptr: 类型安全的空指针
// - NULL 本质是 0（宏定义），在重载解析时会有歧义
// - nullptr 是 std::nullptr_t 类型，可以隐式转换为任何指针类型
void func(int)   { std::cout << "  调用 func(int)\n"; }
void func(int*)  { std::cout << "  调用 func(int*)\n"; }

// 强类型枚举（enum class）
// - 不会隐式转换为 int
// - 有自己的作用域
// - 可以指定底层类型
enum class Color : unsigned char {
    Red = 1,
    Green = 2,
    Blue = 3
};

// C++11 的 constexpr：只能用于很简单的函数
// - 函数体只能有一个 return 语句
// - 所有参数和返回值都必须是字面类型
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

// 变参模板 + 递归展开
// C++11 处理变参的方式：递归 + 重载
template <typename T>
T sum(T value) {
    return value;
}

template <typename T, typename... Args>
T sum(T first, Args... rest) {
    return first + sum(rest...);
}

// 变参模板的经典应用：printf 风格的类型安全打印
void print_args() {
    std::cout << "\n";
}

template <typename T, typename... Args>
void print_args(T first, Args... rest) {
    std::cout << first << " ";
    print_args(rest...);
}

// 别名模板（template alias）
// 比 typedef 更灵活，可以带模板参数
template <typename T>
using StringMap = std::unordered_map<std::string, T>;

// ============================================================
// 初始化列表
// ============================================================
class MyContainer {
public:
    MyContainer() = default;
    MyContainer(std::initializer_list<int> list) {
        for (auto it = list.begin(); it != list.end(); ++it) {
            data_.push_back(*it * 2);  // 示例：存入时翻倍
        }
    }
    void append(std::initializer_list<int> list) {
        for (int v : list) data_.push_back(v);
    }
    const std::vector<int>& data() const { return data_; }
private:
    std::vector<int> data_;
};

// ============================================================
// default / delete
// ============================================================
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    // 移动构造也被隐式删除（声明了拷贝构造/赋值后不会自动生成移动操作）
};

// 阻止特定类型的重载
void accept_int(int) {}
void accept_int(double) = delete;  // 不允许 double 隐式转换

void demo() {
    std::cout << "\n===== 5. 其他重要特性 =====\n";

    std::cout << "--- 5.1 nullptr vs NULL ---\n";
    func(0);        // 调用 func(int)
    func(nullptr);  // 调用 func(int*)
    // func(NULL);  // 在某些编译器上有歧义
    std::cout << "  nullptr 是类型安全的空指针常量\n";

    std::cout << "\n--- 5.2 强类型枚举（enum class） ---\n";
    Color c = Color::Red;
    // int x = c;  // 错误！不能隐式转换
    int x = static_cast<int>(c);  // 必须显式转换
    std::cout << "  Color::Red = " << x << "\n";
    std::cout << "  底层类型大小: " << sizeof(Color) << " 字节\n";

    std::cout << "\n--- 5.3 constexpr（C++11 版） ---\n";
    // 编译期计算
    constexpr int fact5 = factorial(5);
    constexpr int gcd_val = gcd(48, 18);
    static_assert(fact5 == 120, "5! should be 120");
    static_assert(gcd_val == 6, "gcd(48,18) should be 6");
    std::cout << "  factorial(5) = " << fact5 << "（编译期计算）\n";
    std::cout << "  gcd(48, 18) = " << gcd_val << "（编译期计算）\n";

    std::cout << "\n--- 5.4 变参模板（Variadic Templates） ---\n";
    int s = sum(1, 2, 3, 4, 5);
    std::cout << "  sum(1,2,3,4,5) = " << s << "\n";
    std::cout << "  print_args: ";
    print_args(1, "hello", 3.14, 'a', true);

    std::cout << "\n--- 5.5 别名模板 ---\n";
    StringMap<int> ages;
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    std::cout << "  StringMap<int> 就是 unordered_map<string, int>\n";
    std::cout << "  Alice's age = " << ages["Alice"] << "\n";

    std::cout << "\n--- 5.6 初始化列表与统一初始化 ---\n";
    MyContainer mc = {1, 2, 3, 4, 5};  // 使用 initializer_list 构造
    std::cout << "  mc.data() = [";
    for (size_t i = 0; i < mc.data().size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << mc.data()[i];
    }
    std::cout << "]\n";
    mc.append({6, 7, 8});
    std::cout << "  append 后 size = " << mc.data().size() << "\n";

    // 统一初始化的好处：可以防止"最令人烦恼的解析"(most vexing parse)
    std::vector<int> v1(10, 5);  // 10 个元素，每个值为 5
    std::vector<int> v2{10, 5};  // 2 个元素：10 和 5
    std::cout << "  v1(size, value) 大小: " << v1.size() << "\n";
    std::cout << "  v2{list} 大小: " << v2.size() << "\n";

    std::cout << "\n--- 5.7 default / delete ---\n";
    NonCopyable obj;
    // NonCopyable obj2 = obj;  // 错误！拷贝构造被删除
    // accept_int(3.14);       // 错误！double 版本被删除
    std::cout << "  NonCopyable 类禁止拷贝\n";
    std::cout << "  accept_int(3.14) 被禁止（防止隐式转换）\n";

    std::cout << "\n--- 5.8 std::array ---\n";
    std::array<int, 5> arr = {3, 1, 4, 1, 5};
    std::sort(arr.begin(), arr.end());
    std::cout << "  std::array 排序后: ";
    for (int n : arr) std::cout << n << " ";
    std::cout << "\n";
    std::cout << "  std::array 大小固定，栈上分配，比原生数组更安全\n";
}

} // namespace section5_misc

// ============================================================================
// 6. 线程支持库
// ============================================================================
namespace section6_threading {

// C++11 首次将并发纳入标准库
// 主要组件：
// - std::thread: 线程
// - std::mutex / std::recursive_mutex / std::timed_mutex: 互斥锁
// - std::lock_guard / std::unique_lock: RAII 锁管理
// - std::condition_variable: 条件变量
// - std::future / std::promise / std::async: 异步任务
// - std::atomic: 原子操作

std::mutex g_mutex;
int g_counter = 0;

void increment(int id, int times) {
    for (int i = 0; i < times; i++) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_counter++;
        // 模拟一些工作
    }
    std::lock_guard<std::mutex> lock(g_mutex);
    std::cout << "  线程 " << id << " 完成，当前计数: " << g_counter << "\n";
}

// 使用 promise/future 传递结果
int calculate_sum(int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += i;
    }
    return sum;
}

void demo() {
    std::cout << "\n===== 6. 线程支持库 =====\n";

    std::cout << "--- 6.1 std::thread + mutex ---\n";
    g_counter = 0;
    std::thread t1(increment, 1, 10000);
    std::thread t2(increment, 2, 10000);
    std::thread t3(increment, 3, 10000);

    t1.join();
    t2.join();
    t3.join();
    std::cout << "  最终计数: " << g_counter << "（期望 30000）\n";

    std::cout << "\n--- 6.2 std::async + future ---\n";
    // 异步执行任务
    std::future<int> f1 = std::async(std::launch::async, calculate_sum, 1, 100);
    std::future<int> f2 = std::async(std::launch::async, calculate_sum, 101, 200);

    int total = f1.get() + f2.get();
    std::cout << "  1 到 200 的和: " << total << "（期望 20100）\n";

    std::cout << "\n--- 6.3 std::atomic ---\n";
    std::atomic<int> atomic_counter{0};
    auto atomic_increment = [&atomic_counter](int times) {
        for (int i = 0; i < times; i++) {
            atomic_counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::thread ta1(atomic_increment, 10000);
    std::thread ta2(atomic_increment, 10000);
    ta1.join();
    ta2.join();
    std::cout << "  原子计数: " << atomic_counter.load() << "（期望 20000，无锁）\n";
}

} // namespace section6_threading

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  C++11 核心特性深度解析\n";
    std::cout << "  现代 C++ 的基石 —— 理解这些特性是掌握后续版本的前提\n";
    std::cout << "============================================================\n";

    section1_auto_decltype::demo();
    section2_move_semantics::demo();
    section3_lambda::demo();
    section4_smart_ptr::demo();
    section5_misc::demo();
    section6_threading::demo();

    std::cout << "\n============================================================\n";
    std::cout << "  C++11 总结：引入了类型推导、移动语义、lambda、智能指针、\n";
    std::cout << "  变参模板、线程库等核心特性，彻底改变了 C++ 的编程方式。\n";
    std::cout << "============================================================\n";

    return 0;
}
