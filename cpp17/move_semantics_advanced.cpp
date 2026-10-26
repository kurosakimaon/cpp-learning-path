/**
 * @file move_semantics_advanced.cpp
 * @brief C++ 高级移动语义与值类别深度解析
 *
 * 知识点覆盖：
 * 1. 值类别体系详解：lvalue / rvalue / xvalue / prvalue / glvalue
 * 2. 引用折叠规则（Reference Collapsing）
 * 3. 完美转发（Perfect Forwarding）与 std::forward
 * 4. 移动构造与移动赋值的优化：noexcept、自赋值检查、移后状态
 * 5. 返回值优化：RVO / NRVO / 复制消除（Copy Elision）
 * 6. emplace vs push_back：何时用哪个，原理对比
 * 7. 右值引用与函数重载决议
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic -fno-elide-constructors move_semantics_advanced.cpp -o move_semantics_advanced
 * （-fno-elide-constructors 只关闭可选的复制消除（如 NRVO）；
 *    C++17 对返回纯右值的强制复制消除不受影响）
 */

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <memory>
#include <cstring>
#include <algorithm>
#include <chrono>

// ============================================================
// 辅助类：带追踪的资源类，用于观察构造/析构/移动/拷贝
// ============================================================

struct TrackedObject {
    std::string name;
    int* data;       // 动态分配的资源
    size_t size;

    // 默认构造
    TrackedObject() : name("default"), data(nullptr), size(0) {
        std::cout << "  [" << name << "] 默认构造\n";
    }

    // 普通构造
    explicit TrackedObject(std::string n, size_t s = 10)
        : name(std::move(n)), size(s)
    {
        data = new int[s];
        std::fill(data, data + s, 0);
        std::cout << "  [" << name << "] 普通构造 (size=" << s << ")\n";
    }

    // 拷贝构造
    TrackedObject(const TrackedObject& other)
        : name(other.name + "_copy"), size(other.size)
    {
        data = new int[size];
        std::copy(other.data, other.data + size, data);
        std::cout << "  [" << name << "] 拷贝构造 (从 " << other.name << ")\n";
    }

    // 拷贝赋值
    TrackedObject& operator=(const TrackedObject& other) {
        if (this != &other) {
            // 先分配新资源，成功后再释放旧资源，避免 new 抛异常后留下悬空指针
            int* fresh = new int[other.size];
            std::copy(other.data, other.data + other.size, fresh);

            delete[] data;
            data = fresh;
            name = other.name + "_copy";
            size = other.size;
            std::cout << "  [" << name << "] 拷贝赋值 (从 " << other.name << ")\n";
        }
        return *this;
    }

    // 移动构造（noexcept 很重要！）
    TrackedObject(TrackedObject&& other) noexcept
        : name(std::move(other.name)),
          data(other.data),
          size(other.size)
    {
        // 窃取资源，将源对象置为空状态。
        // 注意：noexcept 函数里不能再做可能分配内存的操作（例如字符串拼接），
        // 否则抛异常会直接 std::terminate。
        other.data = nullptr;
        other.size = 0;
        std::cout << "  [" << name << "] 移动构造\n";
    }

    // 移动赋值
    TrackedObject& operator=(TrackedObject&& other) noexcept {
        if (this != &other) {
            // 释放自己的资源
            delete[] data;

            // 窃取资源
            name = std::move(other.name);
            data = other.data;
            size = other.size;

            // 置空源对象
            other.data = nullptr;
            other.size = 0;

            std::cout << "  [" << name << "] 移动赋值\n";
        }
        return *this;
    }

    // 析构
    ~TrackedObject() {
        if (data) {
            delete[] data;
            std::cout << "  [" << name << "] 析构 (释放资源)\n";
        } else {
            std::cout << "  [" << name << "] 析构 (空壳)\n";
        }
    }
};

// ============================================================
// 第一部分：值类别体系详解
// ============================================================

namespace value_categories {

// C++ 表达式的值类别体系：
//
//         expression
//             |
//     +-------+-------+
//     |               |
//   glvalue         rvalue
//     |               |
//  +--+--+         +--+--+
//  |     |         |     |
// lvalue  xvalue  xvalue prvalue
//
// lvalue（左值）：有身份，不能直接移动
//   - 变量名、函数名、数据成员
//   - 返回左值引用的函数调用 (T&)
//   - 解引用表达式 (*p)
//   - 下标表达式 (a[i]，数组的情况)
//   - 注意：用 std::move(lvalue) 可以把它转成 xvalue 再移动
//
// prvalue（纯右值）：没有身份，可以被移动
//   - 字面量 (42, 3.14)；字符串字面量 "hello" 是数组左值，不是 prvalue
//   - 返回非引用的函数调用
//   - 临时对象表达式 (Foo(1, 2))
//   - 算术/逻辑/比较表达式的结果
//
// xvalue（将亡值，"expiring" value）：有身份，可以被移动
//   - 返回右值引用的函数调用 (T&&)
//   - std::move(x) 的结果
//   - 指向数据成员的右值引用
//
// glvalue（广义左值）= lvalue + xvalue（"generalized" lvalue）
// rvalue（右值）= xvalue + prvalue
//
// 判断口诀：
//   - 能不能取地址？ 能 -> lvalue
//   - 有没有名字？   有 -> lvalue
//   - 具名对象 -> lvalue
//   - 纯右值临时对象（未经过 std::move）-> prvalue
//   - std::move() 的结果 -> xvalue

// 1.1 左值 vs 右值的基本区分
void demo_lvalue_vs_rvalue() {
    std::cout << "===== 1. 值类别体系 =====\n";
    std::cout << "--- 1.1 左值 vs 右值 ---\n";

    int x = 42;  // x 是左值，42 是右值

    // 左值引用只能绑定左值（非 const 的情况）
    int& lref = x;       // OK，x 是左值
    // int& lref2 = 42;  // 错误！42 是右值

    // const 左值引用可以绑定右值（延长生命周期）
    const int& clref = 42;  // OK

    // 右值引用只能绑定右值
    int&& rref = 100;       // OK，100 是右值
    // int&& rref2 = x;     // 错误！x 是左值

    // std::move 将左值转为右值（具体说是 xvalue）
    int&& rref3 = std::move(x);  // OK，std::move(x) 是右值

    std::cout << "x = " << x << "\n";
    std::cout << "lref = " << lref << "\n";
    std::cout << "clref = " << clref << "\n";
    std::cout << "rref = " << rref << "\n";
    std::cout << "rref3 (std::move(x)) = " << rref3 << "\n";
}

// 1.2 使用 type traits 判断值类别
template <typename T>
void print_value_category(const char* expr_name) {
    std::cout << "  " << expr_name << ": ";
    if (std::is_lvalue_reference_v<T>) {
        std::cout << "左值 (lvalue)";
    } else if (std::is_rvalue_reference_v<T>) {
        std::cout << "右值 (rvalue/xvalue)";
    } else {
        std::cout << "纯右值 (prvalue)";
    }
    std::cout << " (decltype 类型: " << typeid(T).name() << ")\n";
}

// 注意：decltype((expr)) 可以保留表达式的值类别
// - 如果 expr 是左值，decltype((expr)) 是 T&
// - 如果 expr 是 xvalue，decltype((expr)) 是 T&&
// - 如果 expr 是 prvalue，decltype((expr)) 是 T

void demo_value_category_detection() {
    std::cout << "\n--- 1.2 值类别检测 ---\n";

    int x = 42;
    int* p = &x;
    TrackedObject obj("test");

    std::cout << "使用 decltype((expr)) 检测值类别:\n";

    // 变量名：左值 -> T&
    print_value_category<decltype((x))>("(x)");

    // 解引用：左值 -> T&
    print_value_category<decltype((*p))>("(*p)");

    // 字面量：纯右值 -> T
    print_value_category<decltype((42))>("(42)");

    // std::move(x)：将亡值 -> T&&
    print_value_category<decltype((std::move(x)))>("(std::move(x))");

    // 函数调用（返回值）：纯右值 -> T
    print_value_category<decltype((TrackedObject("temp")))>("(TrackedObject())");

    // 成员访问
    print_value_category<decltype((obj.name))>("(obj.name)");
}

// 1.3 xvalue 的深入理解
void demo_xvalue() {
    std::cout << "\n--- 1.3 xvalue（将亡值）深入 ---\n";

    TrackedObject obj("original");
    std::cout << "std::move 之前: obj.name = " << obj.name << "\n";

    // std::move 不移动任何东西，它只是将表达式转为右值引用（xvalue）
    TrackedObject&& rref = std::move(obj);
    // 此时 obj 还没有被移动，只是可以被移动了

    std::cout << "std::move 之后（还没真正移动）: obj.name = " << obj.name << "\n";
    std::cout << "rref.name = " << rref.name << "\n";

    // 真正的移动发生在构造/赋值时
    TrackedObject moved = std::move(obj);  // 这里才真正移动
    std::cout << "移动构造之后: obj.name = " << obj.name << "\n";
    std::cout << "moved.name = " << moved.name << "\n";
}

} // namespace value_categories


// ============================================================
// 第二部分：引用折叠（Reference Collapsing）
// ============================================================

namespace reference_collapsing {

// 引用折叠规则：
// T& &   -> T&    (左值引用 + 左值引用 = 左值引用)
// T& &&  -> T&    (左值引用 + 右值引用 = 左值引用)
// T&& &  -> T&    (右值引用 + 左值引用 = 左值引用)
// T&& && -> T&&   (右值引用 + 右值引用 = 右值引用)
//
// 口诀：只要有一个 &，结果就是 &；全是 && 才是 &&

// 2.1 显式引用折叠示例
template <typename T>
struct RefTest {
    using lvalue_ref = T&;
    using rvalue_ref = T&&;
};

void demo_basic_collapse() {
    std::cout << "\n===== 2. 引用折叠 =====\n";
    std::cout << "--- 2.1 基本规则 ---\n";

    // T = int& 时
    using T1 = int&;
    static_assert(std::is_same_v<RefTest<T1>::lvalue_ref, int&>);  // int& & -> int&
    static_assert(std::is_same_v<RefTest<T1>::rvalue_ref, int&>);  // int& && -> int&

    // T = int&& 时
    using T2 = int&&;
    static_assert(std::is_same_v<RefTest<T2>::lvalue_ref, int&>);  // int&& & -> int&
    static_assert(std::is_same_v<RefTest<T2>::rvalue_ref, int&&>); // int&& && -> int&&

    std::cout << "T = int&:  T& -> int&,  T&& -> int&\n";
    std::cout << "T = int&&: T& -> int&,  T&& -> int&&\n";
    std::cout << "T = int:   T& -> int&,  T&& -> int&&\n";
    std::cout << "规律：有 & 就坍缩为 &，全 && 才是 &&\n";
}

// 2.2 转发引用（万能引用）与引用折叠
// 这是引用折叠最重要的应用场景
//
// template <typename T>
// void foo(T&& t) { ... }
//
// 当传入左值时，T 被推导为 T&，T& && -> T&（左值引用）
// 当传入右值时，T 被推导为 T，T&& 就是 T&&（右值引用）
//
// 这样，同一个模板可以同时接受左值和右值

template <typename T>
void forward_func(T&& t) {
    // T 的推导结果取决于传入的是左值还是右值
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "  传入左值，T 推导为 " << typeid(T).name()
                  << " (左值引用类型)\n";
    } else {
        std::cout << "  传入右值，T 推导为 " << typeid(T).name()
                  << " (非引用类型)\n";
    }
    std::cout << "    参数 t 的类型: " << typeid(decltype(t)).name() << "\n";
}

void demo_forwarding_reference() {
    std::cout << "\n--- 2.2 转发引用（万能引用） ---\n";

    int x = 42;
    std::cout << "传入左值 x:\n";
    forward_func(x);  // 左值

    std::cout << "传入右值 100:\n";
    forward_func(100);  // 右值

    std::cout << "传入 std::move(x):\n";
    forward_func(std::move(x));  // 右值（xvalue）

    // 注意：转发引用只在 T&& 且 T 是被推导的模板参数时成立
    // 固定类型的右值引用不是转发引用：
    // void foo(int&& x) { ... }  // 这只能绑定右值，不是转发引用
}

} // namespace reference_collapsing


// ============================================================
// 第三部分：完美转发（Perfect Forwarding）
// ============================================================

namespace perfect_forwarding {

// 3.1 为什么需要完美转发？
// 当我们要把参数"原样"转发给另一个函数时，需要保持：
//   - 值类别（左值保持左值，右值保持右值）
//   - const/volatile 限定符
//
// std::forward<T>(t) 的作用：
//   - 如果 T 是左值引用类型，返回左值引用
//   - 如果 T 不是引用类型，返回右值引用
//
// 它与转发引用配合，实现完美转发

// 不完美的转发：丢失值类别
template <typename T>
void bad_forward(T t) {
    // t 始终是左值（有名字的变量都是左值）
    // 即使传入的是右值，到这里也变成了左值
    (void)t;
}

// 完美的转发
template <typename Func, typename... Args>
decltype(auto) call_with_forward(Func&& f, Args&&... args) {
    // std::forward 保持每个参数的值类别
    return std::forward<Func>(f)(std::forward<Args>(args)...);
}

// 演示：构造函数的完美转发
class Wrapper {
    TrackedObject obj_;
public:
    // 完美转发构造：接受任意参数构造内部对象
    template <typename... Args>
    Wrapper(Args&&... args)
        : obj_(std::forward<Args>(args)...)  // 完美转发给 TrackedObject 构造函数
    {
        std::cout << "    Wrapper 构造完成\n";
    }

    const TrackedObject& obj() const { return obj_; }
};

void demo_perfect_forward() {
    std::cout << "\n===== 3. 完美转发 =====\n";
    std::cout << "--- 3.1 完美转发构造函数 ---\n";

    std::cout << "传入左值:\n";
    TrackedObject source("source", 20);
    {
        std::cout << "  构造 Wrapper(左值):\n";
        Wrapper w(source);  // 调用拷贝构造
    }

    std::cout << "\n传入右值（临时对象）:\n";
    {
        std::cout << "  构造 Wrapper(右值):\n";
        Wrapper w(TrackedObject("temp", 30));  // 调用移动构造
    }

    std::cout << "\n传入 std::move(source):\n";
    {
        std::cout << "  构造 Wrapper(std::move):\n";
        Wrapper w(std::move(source));  // 调用移动构造
    }
}

// 3.2 emplace_back 的实现原理
// emplace_back 就是用完美转发在容器内部直接构造对象
// 避免了临时对象的构造和析构

void demo_emplace_vs_push() {
    std::cout << "\n--- 3.2 emplace_back vs push_back ---\n";

    std::cout << "push_back(临时对象):\n";
    {
        std::vector<TrackedObject> vec;
        vec.reserve(10);  // 预分配，避免扩容干扰
        vec.push_back(TrackedObject("push_temp", 10));
        // 过程：构造临时对象 -> 移动构造到 vector -> 析构临时对象
    }

    std::cout << "\nemplace_back(原地构造):\n";
    {
        std::vector<TrackedObject> vec;
        vec.reserve(10);
        vec.emplace_back("emplace_direct", 10);
        // 过程：直接在 vector 内存中构造对象（一次构造）
    }

    std::cout << "\n对比结论:\n";
    std::cout << "  push_back: 构造 + 移动 + 析构临时 = 3 次操作\n";
    std::cout << "  emplace_back: 直接构造 = 1 次操作\n";
    std::cout << "  但对于简单类型（如 int），两者几乎没有区别\n";
}

// 3.3 完美转发的陷阱：过度泛化
// 完美转发构造函数可能意外匹配拷贝/移动构造

class DangerousWrapper {
    std::string name_;
public:
    // 这个构造函数可能比拷贝构造更匹配！
    template <typename T,
              typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, DangerousWrapper>>>
    explicit DangerousWrapper(T&& name)
        : name_(std::forward<T>(name))
    {
        std::cout << "  DangerousWrapper 泛化构造: " << name_ << "\n";
    }

    // 注意：如果没有 enable_if 约束，
    // DangerousWrapper other(wrapper) 可能调用泛化构造而不是拷贝构造！
    // 因为当参数是 non-const lvalue 时，模板推导的 T& 比 const T& 更匹配

    const std::string& name() const { return name_; }
};

void demo_forwarding_pitfall() {
    std::cout << "\n--- 3.3 完美转发陷阱：过度泛化 ---\n";

    DangerousWrapper a("test");
    DangerousWrapper b(a);  // 应该调用拷贝构造
    // 如果没有 enable_if 约束，这里可能调用泛化构造函数！

    std::cout << "通过 std::enable_if 约束，避免与拷贝/移动构造冲突\n";
    std::cout << "约束条件: !std::is_same_v<std::decay_t<T>, DangerousWrapper>\n";
}

} // namespace perfect_forwarding


// ============================================================
// 第四部分：移动语义的最佳实践
// ============================================================

namespace move_best_practices {

// 4.1 noexcept 的重要性
// 移动构造/赋值标记为 noexcept 有两个重要原因：
// 1. 标准库容器（如 vector）在扩容时，如果移动构造是 noexcept，
//    会使用移动而不是拷贝（提供强异常保证）
// 2. 性能：noexcept 可以帮助编译器优化

class NoexceptMoveDemo {
    int* data_;
    size_t size_;
public:
    NoexceptMoveDemo() : data_(nullptr), size_(0) {}

    explicit NoexceptMoveDemo(size_t n)
        : data_(new int[n]), size_(n) {}

    // 移动构造：noexcept
    NoexceptMoveDemo(NoexceptMoveDemo&& other) noexcept
        : data_(other.data_), size_(other.size_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // 移动赋值：noexcept
    NoexceptMoveDemo& operator=(NoexceptMoveDemo&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~NoexceptMoveDemo() { delete[] data_; }
};

// 验证：vector 扩容时是否使用移动
static_assert(std::is_nothrow_move_constructible_v<NoexceptMoveDemo>);
static_assert(std::is_nothrow_move_assignable_v<NoexceptMoveDemo>);

void demo_noexcept_importance() {
    std::cout << "\n===== 4. 移动语义最佳实践 =====\n";
    std::cout << "--- 4.1 noexcept 的重要性 ---\n";

    std::cout << "std::is_nothrow_move_constructible_v<NoexceptMoveDemo> = "
              << std::boolalpha
              << std::is_nothrow_move_constructible_v<NoexceptMoveDemo>
              << "\n";

    std::cout << "vector 扩容时:\n";
    std::cout << "  - 移动构造是 noexcept: 使用移动（高效）\n";
    std::cout << "  - 移动构造可能抛异常: 使用拷贝（保证强异常安全）\n";
    std::cout << "  - 所以自定义类的移动操作应该总是 noexcept\n";
}

// 4.2 移后状态（Moved-from State）
// 被移动的对象应该处于"有效但未指定"的状态
// - 可以安全析构
// - 可以赋值
// - 但不要依赖其具体值

void demo_moved_from_state() {
    std::cout << "\n--- 4.2 移后状态 ---\n";

    std::string s1 = "hello world, this is a long string";
    std::cout << "移动前: s1 = \"" << s1 << "\", size = " << s1.size() << "\n";

    std::string s2 = std::move(s1);
    std::cout << "移动后: s2 = \"" << s2 << "\"\n";
    std::cout << "移动后: s1 = \"" << s1 << "\" (移后状态，值未指定)\n";
    std::cout << "s1.size() = " << s1.size() << " (可能是 0，也可能不是)\n";

    // 移后对象可以做什么？
    s1 = "new value";  // OK：可以赋值
    std::cout << "重新赋值后: s1 = \"" << s1 << "\"\n";
    s1.clear();        // OK：可以 clear
    (void)s1.empty();  // OK：可以查询状态（返回值无副作用，忽略即可）

    std::cout << "\n移后状态的安全操作:\n";
    std::cout << "  - 析构（必须安全）\n";
    std::cout << "  - 赋值（拷贝/移动赋值）\n";
    std::cout << "  - clear / reset / empty 等无前提条件的操作\n";
    std::cout << "不安全的操作:\n";
    std::cout << "  - 依赖具体值的操作（如 front()、operator[]）\n";
    std::cout << "  - 假设它是空的（可能不是）\n";
}

// 4.3 什么时候该用 std::move？
void demo_when_to_move() {
    std::cout << "\n--- 4.3 何时使用 std::move ---\n";

    std::cout << "应该用 std::move 的场景:\n";
    std::cout << "  1. 明确知道对象不再需要了（最后一次使用）\n";
    std::cout << "  2. 函数参数按值传递时，移动到成员变量\n";
    std::cout << "  3. 在容器中插入已知不再使用的对象\n";
    std::cout << "  4. 实现移动构造/赋值时，对成员使用 std::move\n\n";

    std::cout << "不应该用 std::move 的场景:\n";
    std::cout << "  1. return 局部变量时（会阻碍 NRVO）\n";
    std::cout << "  2. 对基本类型使用（int, double 等，移动就是拷贝）\n";
    std::cout << "  3. 不确定对象是否还会被使用时\n";
    std::cout << "  4. 返回纯右值临时对象（C++17 会强制复制消除）\n";
}

// 4.4 按值传递 + 移动的惯用法
class User {
    std::string name_;
    std::string email_;
public:
    // 方案1：按值传递 + 移动
    // 优点：只需写一个函数，左值拷贝+移动，右值移动+移动
    // 缺点：左值情况下比 const& + 拷贝多一次移动
    User(std::string name, std::string email)
        : name_(std::move(name)), email_(std::move(email)) {}

    // 方案2：const& + && 重载
    // 优点：左值情况下少一次移动
    // 缺点：代码量翻倍（2^N 个重载）
    // User(const std::string& name, const std::string& email)
    //     : name_(name), email_(email) {}
    // User(std::string&& name, std::string&& email)
    //     : name_(std::move(name)), email_(std::move(email)) {}
    // ... 还有其他组合，太多了

    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
};

void demo_pass_by_value() {
    std::cout << "\n--- 4.4 按值传递 + 移动惯用法 ---\n";

    std::string name = "Alice";
    std::string email = "alice@example.com";

    std::cout << "左值传入:\n";
    User u1(name, email);  // 拷贝 + 移动
    std::cout << "  name: " << u1.name() << "\n";

    std::cout << "右值传入:\n";
    User u2(std::string("Bob"), std::string("bob@example.com"));  // 移动 + 移动
    std::cout << "  name: " << u2.name() << "\n";

    std::cout << "\n适用场景：参数最终要被存储（如构造函数初始化成员）\n";
    std::cout << "权衡：代码简洁 vs 左值多一次移动\n";
    std::cout << "string 的移动很便宜（几个指针赋值），通常可以接受\n";
}

} // namespace move_best_practices


// ============================================================
// 第五部分：返回值优化（RVO / NRVO）
// ============================================================

namespace rvo_demo {

// 5.1 RVO (Return Value Optimization)
// 当函数返回一个纯右值临时对象时，C++17 要求直接在目标位置构造，
// 不发生拷贝或移动（guaranteed copy elision）

// C++17 起，RVO 是强制性的（guaranteed copy elision）
// 对于纯右值返回值，编译器必须省略拷贝

TrackedObject rvo_function() {
    // 直接返回临时对象：RVO
    return TrackedObject("rvo_obj", 10);
}

// 5.2 NRVO (Named Return Value Optimization)
// 当函数返回一个局部变量时，编译器可以将其构造在返回值地址上
// 这是可选的优化，不保证发生

TrackedObject nrvo_function() {
    TrackedObject obj("nrvo_obj", 20);
    obj.name += "_modified";
    return obj;  // NRVO：直接构造在返回值位置
}

// 5.3 反模式：return std::move(local)
// 这会阻碍 NRVO！因为返回右值引用不能触发 NRVO
TrackedObject bad_return_move() {
    TrackedObject obj("bad_move", 30);
    return std::move(obj);  // 不要这么写！
    // 没有 NRVO 的话，至少还有移动
    // 但如果有 NRVO，std::move 反而导致多一次移动
}

void demo_rvo() {
    std::cout << "\n===== 5. 返回值优化（RVO/NRVO） =====\n";

    std::cout << "--- 5.1 RVO（返回临时对象） ---\n";
    std::cout << "调用 rvo_function():\n";
    {
        TrackedObject obj = rvo_function();
        std::cout << "  最终对象名: " << obj.name << "\n";
    }
    std::cout << "(C++17 起，RVO 是强制保证的)\n";

    std::cout << "\n--- 5.2 NRVO（返回命名局部变量） ---\n";
    std::cout << "调用 nrvo_function():\n";
    {
        TrackedObject obj = nrvo_function();
        std::cout << "  最终对象名: " << obj.name << "\n";
    }
    std::cout << "(NRVO 是可选优化，编译器可能不启用)\n";

    std::cout << "\n--- 5.3 反模式：return std::move(local) ---\n";
    std::cout << "调用 bad_return_move():\n";
    {
        TrackedObject obj = bad_return_move();
        std::cout << "  最终对象名: " << obj.name << "\n";
    }
    std::cout << "(不要在 return 时对局部变量用 std::move，会阻碍 NRVO)\n";
}

// 5.4 复制消除（Copy Elision）的其他场景
void demo_copy_elision() {
    std::cout << "\n--- 5.4 复制消除的其他场景 ---\n";

    std::cout << "场景1: 函数传值参数中的临时对象\n";
    auto take_by_value = [](TrackedObject obj) {
        std::cout << "    函数内部: " << obj.name << "\n";
    };
    take_by_value(TrackedObject("arg_temp", 5));
    // 临时对象直接构造在参数位置，没有拷贝

    std::cout << "\n场景2: throw / catch 中的复制消除\n";
    std::cout << "  抛出异常对象时，也可能发生复制消除\n";

    std::cout << "\n场景3: C++17 纯右值的保证复制消除\n";
    std::cout << "  auto x = T(); // C++17 起，保证零拷贝\n";
    std::cout << "  （T 的构造函数直接在 x 的地址上调用）\n";
}

} // namespace rvo_demo


// ============================================================
// 第六部分：右值引用与函数重载
// ============================================================

namespace overload_resolution {

// 6.1 左值/右值重载
void process(const std::string& s) {
    std::cout << "  process(const string&): 左值/const 右值 -> " << s << "\n";
}

void process(std::string&& s) {
    std::cout << "  process(string&&): 非const右值 -> " << s << "\n";
}

void demo_rvalue_overload() {
    std::cout << "\n===== 6. 右值引用与重载决议 =====\n";
    std::cout << "--- 6.1 左值/右值重载 ---\n";

    std::string s = "hello";
    const std::string cs = "world";

    std::cout << "传入左值 s:\n";
    process(s);  // 调用 const& 版本

    std::cout << "传入 const 左值 cs:\n";
    process(cs);  // 调用 const& 版本

    std::cout << "传入右值（临时）:\n";
    process(std::string("temp"));  // 调用 && 版本

    std::cout << "传入 std::move(s):\n";
    process(std::move(s));  // 调用 && 版本

    std::cout << "传入 std::move(cs):\n";
    process(std::move(cs));  // const 右值 -> 调用 const& 版本（没有 const&& 重载）
}

// 6.2 引用限定符（Ref-qualified）成员函数
// 可以根据 *this 是左值还是右值来选择不同的成员函数重载

class DataHolder {
    std::vector<int> data_;
public:
    DataHolder(std::initializer_list<int> il) : data_(il) {}

    // 左值版本：返回 const 引用
    const std::vector<int>& data() const& {
        std::cout << "  data() const& 被调用（左值/const对象）\n";
        return data_;
    }

    // 右值版本：返回值（移动）
    std::vector<int> data() && {
        std::cout << "  data() && 被调用（右值对象，移动数据）\n";
        return std::move(data_);
    }
};

void demo_ref_qualified() {
    std::cout << "\n--- 6.2 引用限定符成员函数 ---\n";

    DataHolder dh{1, 2, 3, 4, 5};

    std::cout << "左值对象调用 data():\n";
    const auto& ref = dh.data();  // 左值版本，返回引用
    std::cout << "  结果大小: " << ref.size() << "\n";

    std::cout << "右值对象调用 data():\n";
    auto moved = DataHolder{6, 7, 8}.data();  // 右值版本，移动数据
    std::cout << "  结果大小: " << moved.size() << "\n";

    std::cout << "\n应用场景:\n";
    std::cout << "  - 链式调用中优化临时对象\n";
    std::cout << "  - 避免从临时对象返回引用（悬空引用）\n";
    std::cout << "  - 提供移动语义的 getter\n";
}

} // namespace overload_resolution


// ============================================================
// 第七部分：综合示例 - 高性能字符串缓冲区
// ============================================================

namespace string_buffer {

// 一个支持移动语义的高性能字符串缓冲区
class StringBuffer {
    char* data_;
    size_t size_;
    size_t capacity_;

public:
    // 默认构造：始终保留 1 字节存放 '\0'，避免后面 memcpy/append 读到空指针
    StringBuffer() : data_(new char[1]{'\0'}), size_(0), capacity_(1) {}

    // 从 C 字符串构造
    explicit StringBuffer(const char* str) {
        if (str == nullptr) str = "";
        size_ = std::strlen(str);
        capacity_ = size_ + 1;
        data_ = new char[capacity_];
        std::memcpy(data_, str, size_ + 1);
    }

    // 预分配容量
    explicit StringBuffer(size_t capacity)
        : data_(new char[capacity == 0 ? 1 : capacity]),
          size_(0),
          capacity_(capacity == 0 ? 1 : capacity)
    {
        data_[0] = '\0';
    }

    // 析构
    ~StringBuffer() {
        delete[] data_;
    }

    // 拷贝构造
    StringBuffer(const StringBuffer& other)
        : data_(nullptr),
          size_(other.size_),
          capacity_(std::max(other.capacity_, other.size_ + 1))
    {
        data_ = new char[capacity_];
        if (other.data_ != nullptr) {
            std::memcpy(data_, other.data_, size_ + 1);
        } else {
            // other 可能是被移动过的对象：空但合法
            size_ = 0;
            data_[0] = '\0';
        }
        std::cout << "  StringBuffer 拷贝构造 (" << size_ << " bytes)\n";
    }

    // 移动构造（noexcept）
    StringBuffer(StringBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        std::cout << "  StringBuffer 移动构造 (O(1))\n";
    }

    // 拷贝赋值：copy-and-swap，天然提供强异常安全
    StringBuffer& operator=(const StringBuffer& other) {
        if (this != &other) {
            StringBuffer temp(other);
            std::swap(data_, temp.data_);
            std::swap(size_, temp.size_);
            std::swap(capacity_, temp.capacity_);
            std::cout << "  StringBuffer 拷贝赋值 (" << size_ << " bytes)\n";
        }
        return *this;
    }

    // 移动赋值（noexcept）
    StringBuffer& operator=(StringBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
            std::cout << "  StringBuffer 移动赋值 (O(1))\n";
        }
        return *this;
    }

    // 追加字符串
    void append(const char* str) {
        if (str == nullptr) return;
        size_t len = std::strlen(str);
        if (len == 0) return;
        if (size_ + len + 1 > capacity_) {
            // 扩容
            size_t new_cap = std::max(capacity_ * 2, size_ + len + 1);
            char* new_data = new char[new_cap];
            if (size_ > 0) {
                std::memcpy(new_data, data_, size_);
            }
            delete[] data_;
            data_ = new_data;
            capacity_ = new_cap;
        }
        std::memcpy(data_ + size_, str, len + 1);
        size_ += len;
    }

    // 拼接 + 移动的高效实现
    StringBuffer operator+(const StringBuffer& other) const {
        StringBuffer result(size_ + other.size_ + 1);
        result.append(c_str());
        result.append(other.c_str());
        return result;  // NRVO
    }

    const char* c_str() const { return data_ ? data_ : ""; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
};

// 工厂函数（利用 RVO）
StringBuffer make_greeting(const char* name) {
    StringBuffer buf("Hello, ");
    buf.append(name);
    buf.append("!");
    return buf;  // NRVO
}

void demo_string_buffer() {
    std::cout << "\n===== 7. 综合示例：高性能字符串缓冲区 =====\n";

    std::cout << "构造字符串:\n";
    StringBuffer s1 = make_greeting("World");
    std::cout << "  结果: " << s1.c_str() << " (size=" << s1.size() << ")\n";

    std::cout << "\n移动构造:\n";
    StringBuffer s2 = std::move(s1);
    std::cout << "  s2: " << s2.c_str() << "\n";
    std::cout << "  s1.size() = " << s1.size() << " (移后状态)\n";

    std::cout << "\n字符串拼接:\n";
    StringBuffer a("Hello, ");
    StringBuffer b("World!");
    StringBuffer c = a + b;
    std::cout << "  拼接结果: " << c.c_str() << "\n";

    std::cout << "\n放入 vector:\n";
    std::vector<StringBuffer> vec;
    vec.reserve(3);
    vec.emplace_back("first");
    vec.emplace_back("second");
    vec.emplace_back("third");
    std::cout << "  vector 中有 " << vec.size() << " 个字符串\n";
}

} // namespace string_buffer


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++ 高级移动语义与值类别深度解析\n";
    std::cout << "============================================\n";

    value_categories::demo_lvalue_vs_rvalue();
    value_categories::demo_value_category_detection();
    value_categories::demo_xvalue();

    reference_collapsing::demo_basic_collapse();
    reference_collapsing::demo_forwarding_reference();

    perfect_forwarding::demo_perfect_forward();
    perfect_forwarding::demo_emplace_vs_push();
    perfect_forwarding::demo_forwarding_pitfall();

    move_best_practices::demo_noexcept_importance();
    move_best_practices::demo_moved_from_state();
    move_best_practices::demo_when_to_move();
    move_best_practices::demo_pass_by_value();

    rvo_demo::demo_rvo();
    rvo_demo::demo_copy_elision();

    overload_resolution::demo_rvalue_overload();
    overload_resolution::demo_ref_qualified();

    string_buffer::demo_string_buffer();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
