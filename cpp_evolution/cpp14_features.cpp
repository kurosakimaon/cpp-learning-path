/**
 * @file cpp14_features.cpp
 * @brief C++14 特性解析 —— C++11 的完善与小步演进
 *
 * C++14 是一个"小版本"，主要是对 C++11 的完善和补充，
 * 但其中一些特性大大提升了代码的表达力和简洁性。
 *
 * 主要特性：
 * 1. 泛型 lambda (Generic Lambda)
 * 2. 函数返回类型推导 (Return Type Deduction for Functions)
 * 3. 变量模板 (Variable Templates)
 * 4. 广义 constexpr (Relaxed constexpr)
 * 5. 二进制字面量 (Binary Literals)
 * 6. 数字分隔符 (Digit Separators)
 * 7. std::make_unique
 * 8. [[deprecated]] 属性
 * 9. lambda 初始化捕获 (Lambda Capture Initializers)
 * 10. 变量模板与类型特征 (type traits 变量模板)
 *
 * 编译命令: g++ -std=c++14 -Wall -Wextra -pedantic cpp14_features.cpp -o cpp14_features
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <complex>
#include <numeric>

// ============================================================================
// 1. 泛型 Lambda (Generic Lambda)
// ============================================================================
namespace section1_generic_lambda {

// C++11 的 lambda 参数必须指定具体类型
// C++14 允许使用 auto 作为 lambda 参数类型
// 本质：闭包类的 operator() 是模板函数

void demo() {
    std::cout << "\n===== 1. 泛型 Lambda =====\n";

    // 泛型 lambda：参数类型用 auto
    auto add = [](auto a, auto b) { return a + b; };

    std::cout << "add(1, 2) = " << add(1, 2) << "  (int)\n";
    std::cout << "add(1.5, 2.3) = " << add(1.5, 2.3) << "  (double)\n";
    std::cout << "add(std::string(\"hello\"), \" world\") = "
              << add(std::string("hello"), " world") << "  (string)\n";

    // 更复杂的例子：通用的比较器
    auto greater = [](const auto& a, const auto& b) { return a > b; };

    std::vector<int> vi = {5, 2, 8, 1, 9};
    std::sort(vi.begin(), vi.end(), greater);
    std::cout << "\nint 降序: ";
    for (int n : vi) std::cout << n << " ";
    std::cout << "\n";

    std::vector<std::string> vs = {"banana", "apple", "cherry", "date"};
    std::sort(vs.begin(), vs.end(), greater);
    std::cout << "string 降序: ";
    for (const auto& s : vs) std::cout << s << " ";
    std::cout << "\n";

    // 与 C++11 的对比：C++11 需要用模板函数或仿函数
    // 泛型 lambda 让代码更简洁

    // 进阶：多参数泛型 lambda + 完美转发
    auto forward_call = [](auto&& func, auto&&... args) -> decltype(auto) {
        return std::forward<decltype(func)>(func)(
            std::forward<decltype(args)>(args)...);
    };

    auto multiply = [](int a, int b) { return a * b; };
    int result = forward_call(multiply, 6, 7);
    std::cout << "\nforward_call(multiply, 6, 7) = " << result << "\n";

    // 实际应用：通用的日志装饰器
    auto logged = [](auto&& func, const char* name) {
        return [func = std::forward<decltype(func)>(func), name](auto&&... args) {
            std::cout << "  [LOG] 调用 " << name << "\n";
            return func(std::forward<decltype(args)>(args)...);
        };
    };

    auto logged_add = logged([](int x, int y) { return x + y; }, "add");
    std::cout << "logged_add(3, 4) = " << logged_add(3, 4) << "\n";
}

} // namespace section1_generic_lambda

// ============================================================================
// 2. 函数返回类型推导
// ============================================================================
namespace section2_return_deduction {

// C++11: 只能用尾置返回类型 (-> decltype(...))
// C++14: 普通函数也可以用 auto 推导返回类型

// 简单情况
auto add_int(int a, int b) {
    return a + b;
}

// 多个 return 语句必须返回相同类型
auto max_value(int a, int b) {
    if (a > b) return a;
    else return b;
}

// 模板函数的返回类型推导（C++11 需要尾置返回类型，C++14 简化了）
template <typename T, typename U>
auto add_template(T a, U b) {
    return a + b;
}

// decltype(auto)：保留引用和值类别
// 与 auto 的区别同变量推导：auto 去掉引用/顶层const，decltype(auto) 保留
int global_value = 42;

auto get_value_auto() {
    return global_value;  // 返回 int
}

decltype(auto) get_value_decltype_auto() {
    return global_value;  // 返回 int（因为 global_value 是左值表达式? 不，它是id表达式）
    // 注意：return expr 中，
    // - 如果 expr 是无括号的 id 表达式 -> decltype(entity) 规则 -> int
    // - 如果是 (expr) -> decltype(expr) 规则 -> int&（如果是左值）
}

decltype(auto) get_value_ref() {
    return (global_value);  // 返回 int&（带括号，按左值表达式处理）
}

// 实际应用：完美转发工厂函数
template <typename T, typename... Args>
auto make_unique_cpp11_style(Args&&... args)
    -> std::unique_ptr<T>  // C++11 写法
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
decltype(auto) make_unique_cpp14_style(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// 进阶：返回类型推导与递归
// C++14 允许递归函数使用返回类型推导
// 但必须先有一个 return 语句让编译器推导出类型
template <typename T>
auto sum_all(T value) {
    return value;
}

template <typename T, typename... Rest>
auto sum_all(T first, Rest... rest) {
    return first + sum_all(rest...);
}

void demo() {
    std::cout << "\n===== 2. 函数返回类型推导 =====\n";

    std::cout << "--- 2.1 普通函数 ---\n";
    auto r1 = add_int(3, 4);
    auto r2 = max_value(10, 20);
    std::cout << "add_int(3, 4) = " << r1 << "  类型由编译器推导\n";
    std::cout << "max_value(10, 20) = " << r2 << "\n";

    std::cout << "\n--- 2.2 模板函数 ---\n";
    auto r3 = add_template(1, 2.5);
    auto r4 = add_template(std::string("hello "), std::string("world"));
    std::cout << "add_template(1, 2.5) = " << r3 << "\n";
    std::cout << "add_template(string, string) = " << r4 << "\n";

    std::cout << "\n--- 2.3 auto vs decltype(auto) ---\n";
    auto v1 = get_value_auto();
    v1 = 100;
    std::cout << "get_value_auto() 返回值类型，修改后 global_value = "
              << global_value << "（不受影响）\n";

    auto v2 = get_value_ref();  // v2 是 int（值捕获）
    v2 = 200;
    std::cout << "用 auto 接收 get_value_ref() 返回值，global_value = "
              << global_value << "（不受影响，因为被拷贝了）\n";

    decltype(auto) v3 = get_value_ref();  // v3 是 int&
    v3 = 200;
    std::cout << "用 decltype(auto) 接收，global_value = "
              << global_value << "（v3 是引用，修改生效）\n";

    // 恢复
    global_value = 42;

    std::cout << "\n--- 2.4 递归函数推导 ---\n";
    auto total = sum_all(1, 2, 3, 4, 5);
    std::cout << "sum_all(1, 2, 3, 4, 5) = " << total << "\n";

    // C++11 vs C++14 对比：尾置返回类型 vs auto
    std::cout << "\n--- 2.5 C++11 尾置返回 vs C++14 auto ---\n";
    std::cout << "C++11: template<typename T, typename U>\n";
    std::cout << "       auto add(T a, U b) -> decltype(a + b) { return a + b; }\n";
    std::cout << "C++14: template<typename T, typename U>\n";
    std::cout << "       auto add(T a, U b) { return a + b; }\n";
    std::cout << "更加简洁，尤其对于复杂的返回类型\n";
}

} // namespace section2_return_deduction

// ============================================================================
// 3. 变量模板 (Variable Templates)
// ============================================================================
namespace section3_variable_templates {

// C++11 只有函数模板和类模板
// C++14 引入变量模板：模板化的变量

// 经典例子：pi 常量
template <typename T>
constexpr T pi = T(3.1415926535897932384626433832795);

// 计算圆面积
template <typename T>
T circle_area(T radius) {
    return pi<T> * radius * radius;
}

// 类型特征的变量模板（C++14 标准库大量使用）
// 标准库中：std::is_same_v<T, U> 就是变量模板
// 它等价于 std::is_same<T, U>::value

// 自定义类型特征变量模板
template <typename T>
constexpr bool is_integral_v = std::is_integral<T>::value;

template <typename T>
constexpr bool is_floating_point_v = std::is_floating_point<T>::value;

// 更复杂的例子：配置参数
template <typename T>
struct TypeConfig {
    static constexpr size_t buffer_size = sizeof(T) * 16;
    static constexpr bool is_trivial = std::is_trivial<T>::value;
};

// 变量模板版本
template <typename T>
constexpr size_t buffer_size_v = TypeConfig<T>::buffer_size;

// 进阶：变量模板特化
template <typename T>
constexpr const char* type_name_v = "unknown";

template <>
constexpr const char* type_name_v<int> = "int";

template <>
constexpr const char* type_name_v<double> = "double";

template <>
constexpr const char* type_name_v<std::string> = "std::string";

// 变量模板与默认值
template <typename T, T Default = T{}>
constexpr T default_value_v = Default;

void demo() {
    std::cout << "\n===== 3. 变量模板 =====\n";

    std::cout << "--- 3.1 基本用法 ---\n";
    std::cout << "pi<int>    = " << pi<int> << "\n";
    std::cout << "pi<float>  = " << pi<float> << "\n";
    std::cout << "pi<double> = " << pi<double> << "\n";

    std::cout << "圆面积 (r=5.0, double): " << circle_area(5.0) << "\n";
    std::cout << "圆面积 (r=5, int):     " << circle_area(5) << "\n";

    std::cout << "\n--- 3.2 类型特征变量模板 ---\n";
    std::cout << "is_integral_v<int>     = " << std::boolalpha
              << is_integral_v<int> << "\n";
    std::cout << "is_integral_v<double>  = "
              << is_integral_v<double> << "\n";
    std::cout << "标准库的 std::is_same_v<int, int> = "
              << std::is_same_v<int, int> << "\n";
    std::cout << "C++14 起 _v 后缀的变量模板大量出现在 <type_traits> 中\n";

    std::cout << "\n--- 3.3 特化变量模板 ---\n";
    std::cout << "type_name_v<int>          = " << type_name_v<int> << "\n";
    std::cout << "type_name_v<double>       = " << type_name_v<double> << "\n";
    std::cout << "type_name_v<std::string>  = " << type_name_v<std::string> << "\n";
    std::cout << "type_name_v<char>         = " << type_name_v<char> << "\n";

    std::cout << "\n--- 3.4 与 C++11 的对比 ---\n";
    std::cout << "C++11 写法：std::is_same<T, U>::value\n";
    std::cout << "C++14 写法：std::is_same_v<T, U>\n";
    std::cout << "省去了 ::value，代码更简洁，编译也可能更快\n";

    std::cout << "\n--- 3.5 实际应用：配置系统 ---\n";
    std::cout << "buffer_size_v<int>    = " << buffer_size_v<int> << " 字节\n";
    std::cout << "buffer_size_v<double> = " << buffer_size_v<double> << " 字节\n";
    std::cout << "buffer_size_v<char>   = " << buffer_size_v<char> << " 字节\n";
}

} // namespace section3_variable_templates

// ============================================================================
// 4. 广义 constexpr (Relaxed constexpr)
// ============================================================================
namespace section4_relaxed_constexpr {

// C++11 的 constexpr 函数限制非常多：
// - 函数体只能有一条 return 语句
// - 不能有局部变量、循环、if 等
//
// C++14 大幅放松了限制：
// - 可以有多个 return
// - 可以有局部变量（必须初始化）
// - 可以有循环、if、switch
// - 可以有 void 函数（不返回值，但可以修改引用参数）
// - 可以有 constexpr lambda

// C++14 constexpr：可以有循环和局部变量
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// 可以有 if
constexpr bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// constexpr lambda（C++17 才正式成为标准，但部分编译器在 C++14 就支持）
// C++14 的 constexpr 函数可以更复杂
constexpr int sum_range(int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += i;
    }
    return sum;
}

// 编译期字符串处理
constexpr int string_length(const char* str) {
    return *str == '\0' ? 0 : 1 + string_length(str + 1);
}

// 可以修改通过引用传递的参数（只要是 constexpr 上下文中的变量）
constexpr void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

// 编译期排序（简化版冒泡排序）
template <size_t N>
constexpr void sort_array(int (&arr)[N]) {
    for (size_t i = 0; i < N - 1; i++) {
        for (size_t j = 0; j < N - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

void demo() {
    std::cout << "\n===== 4. 广义 constexpr =====\n";

    std::cout << "--- 4.1 带循环的 constexpr ---\n";
    constexpr int fact10 = factorial(10);
    static_assert(fact10 == 3628800, "10! should be 3628800");
    std::cout << "factorial(10) = " << fact10 << "（编译期计算，含循环）\n";

    constexpr bool prime_17 = is_prime(17);
    constexpr bool prime_15 = is_prime(15);
    static_assert(prime_17 == true, "17 is prime");
    static_assert(prime_15 == false, "15 is not prime");
    std::cout << "is_prime(17) = " << std::boolalpha << prime_17 << "\n";
    std::cout << "is_prime(15) = " << prime_15 << "\n";

    std::cout << "\n--- 4.2 编译期数组排序 ---\n";
    constexpr int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    // 注意：C++14 中 constexpr 函数不能直接修改全局数组
    // 但可以在 constexpr 上下文中创建和修改局部数组
    struct Sorter {
        int data[9];
        constexpr Sorter() : data{5, 2, 8, 1, 9, 3, 7, 4, 6} {
            // 冒泡排序
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8 - i; j++) {
                    if (data[j] > data[j + 1]) {
                        int temp = data[j];
                        data[j] = data[j + 1];
                        data[j + 1] = temp;
                    }
                }
            }
        }
    };
    constexpr Sorter sorted;
    std::cout << "编译期排序结果: ";
    for (int i = 0; i < 9; i++) {
        std::cout << sorted.data[i] << " ";
    }
    std::cout << "\n";
    static_assert(sorted.data[0] == 1, "first element should be 1");
    static_assert(sorted.data[8] == 9, "last element should be 9");

    std::cout << "\n--- 4.3 C++11 vs C++14 constexpr 对比 ---\n";
    std::cout << "C++11: 只能有一个 return，不能有局部变量、循环\n";
    std::cout << "C++14: 允许多 return、局部变量、循环、if/switch\n";
    std::cout << "这使得 constexpr 函数的实用性大大增强\n";

    std::cout << "\n--- 4.4 运行期也可以调用 constexpr 函数 ---\n";
    int n;
    n = 7;  // 运行期变量
    int result = factorial(n);  // 运行期计算
    std::cout << "运行期调用 factorial(" << n << ") = " << result << "\n";
    std::cout << "constexpr 函数在编译期上下文中编译期计算，\n";
    std::cout << "在运行期上下文中运行期执行，一函数两用\n";
}

} // namespace section4_relaxed_constexpr

// ============================================================================
// 5. 二进制字面量、数字分隔符、[[deprecated]]
// ============================================================================
namespace section5_literals_and_attributes {

// 二进制字面量：0b 或 0B 前缀
// 数字分隔符：单引号 ' 用于数字可读性

// [[deprecated]] 属性：标记已废弃的函数/类/变量
[[deprecated("请使用 new_function() 替代")]]
void old_function() {
    std::cout << "  这是旧函数\n";
}

void new_function() {
    std::cout << "  这是新函数\n";
}

class [[deprecated("使用 NewClass 替代")]] OldClass {
public:
    void foo() {}
};

class NewClass {
public:
    void foo() {}
};

// 变量模板也可以废弃
template <typename T>
[[deprecated("使用 pi_v2 替代")]]
constexpr T pi_old = T(3.1415926);

void demo() {
    std::cout << "\n===== 5. 字面量与属性增强 =====\n";

    std::cout << "--- 5.1 二进制字面量 ---\n";
    int binary1 = 0b101010;     // 42
    int binary2 = 0B1111'0000;  // 240，带数字分隔符
    unsigned int mask = 0b1111'0000'0000'0000;

    std::cout << "0b101010 = " << binary1 << "\n";
    std::cout << "0B1111'0000 = " << binary2 << "\n";
    std::cout << "mask = " << mask << " (0xF000)\n";
    std::cout << "适合硬件编程、位操作等场景\n";

    std::cout << "\n--- 5.2 数字分隔符 ---\n";
    int million = 1'000'000;
    double pi = 3.14159'26535'89793;
    unsigned int hex_num = 0xFF'AA'00'11;
    unsigned long long big = 12'345'678'901'234ULL;

    std::cout << "1'000'000 = " << million << "\n";
    std::cout << "pi = " << pi << "\n";
    std::cout << "0xFF'AA'00'11 = " << std::hex << hex_num << std::dec << "\n";
    std::cout << "大数字: " << big << "\n";
    std::cout << "数字分隔符不影响值，只提高可读性\n";

    std::cout << "\n--- 5.3 [[deprecated]] 属性 ---\n";
    std::cout << "  调用旧函数会产生编译警告（演示中已注释避免警告）\n";
    // old_function();  // 编译时会有 deprecated 警告
    new_function();

    std::cout << "  OldClass 也被标记为废弃\n";
    // OldClass oc;  // 编译警告
    NewClass nc;
    nc.foo();

    std::cout << "\n  [[deprecated]] 可以用于函数、类、变量、枚举等\n";
    std::cout << "  可选地提供废弃原因字符串\n";
}

} // namespace section5_literals_and_attributes

// ============================================================================
// 6. Lambda 初始化捕获（广义捕获）
// ============================================================================
namespace section6_capture_initializers {

// C++11: lambda 只能按值或按引用捕获已有的变量
// C++14: 可以在捕获子句中初始化新变量（也叫广义捕获）
//
// 语法：[var = init] 或 [var = std::move(var)]
// 用途：
// - 移动捕获（move capture）
// - 创建捕获内的新变量
// - 表达式捕获

class MoveOnly {
public:
    MoveOnly(int value) : value_(value) {
        std::cout << "  MoveOnly 构造 (" << value_ << ")\n";
    }
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) noexcept : value_(other.value_) {
        other.value_ = 0;
        std::cout << "  MoveOnly 移动 (" << value_ << ")\n";
    }
    int value() const { return value_; }
private:
    int value_;
};

void demo() {
    std::cout << "\n===== 6. Lambda 初始化捕获 =====\n";

    std::cout << "--- 6.1 移动捕获 ---\n";
    MoveOnly mo(42);

    // C++11 无法捕获 move-only 对象（只能用 shared_ptr 等 workaround）
    // C++14 可以用初始化捕获实现 move capture
    auto lambda = [mo_captured = std::move(mo)]() {
        std::cout << "  lambda 中捕获的 MoveOnly 值: " << mo_captured.value() << "\n";
    };
    lambda();

    std::cout << "\n--- 6.2 表达式捕获（创建新变量） ---\n";
    int x = 10, y = 20;

    // 在捕获中创建新变量
    auto sum_lambda = [sum = x + y, product = x * y]() {
        std::cout << "  sum = " << sum << ", product = " << product << "\n";
    };
    sum_lambda();

    std::cout << "\n--- 6.3 成员变量捕获 ---\n";
    struct Widget {
        int id = 42;
        auto get_lambda() {
            // C++14 可以这样捕获成员变量的拷贝
            return [id = id]() {
                std::cout << "  Widget id = " << id << "\n";
            };
            // 对比：C++11 只能 [this] 捕获整个对象指针
        }
    };

    Widget w;
    auto wl = w.get_lambda();
    wl();

    std::cout << "\n--- 6.4 实际应用：延迟计算 ---\n";
    auto lazy_value = [value = std::vector<int>{1, 2, 3, 4, 5}]() {
        return std::accumulate(value.begin(), value.end(), 0);
    };
    std::cout << "  延迟计算结果: " << lazy_value() << "\n";

    std::cout << "\n--- 6.5 C++11 vs C++14 捕获对比 ---\n";
    std::cout << "C++11: [=], [&], [x, &y] —— 只能捕获已有变量\n";
    std::cout << "C++14: [x = expr], [y = std::move(z)] —— 可创建新变量、移动捕获\n";
    std::cout << "广义捕获让 lambda 更灵活，尤其适合 move-only 类型\n";
}

} // namespace section6_capture_initializers

// ============================================================================
// 7. std::make_unique
// ============================================================================
namespace section7_make_unique {

// C++11 有 std::make_shared，但没有 std::make_unique（疏忽了）
// C++14 补上了 std::make_unique

class Widget {
public:
    Widget(int a, int b) : a_(a), b_(b) {
        std::cout << "  Widget(" << a_ << ", " << b_ << ") 构造\n";
    }
    ~Widget() {
        std::cout << "  Widget(" << a_ << ", " << b_ << ") 析构\n";
    }
    int sum() const { return a_ + b_; }
private:
    int a_, b_;
};

void demo() {
    std::cout << "\n===== 7. std::make_unique =====\n";

    std::cout << "--- 7.1 基本用法 ---\n";
    {
        // C++11 写法
        std::unique_ptr<Widget> p1(new Widget(1, 2));
        std::cout << "  C++11 写法: unique_ptr<Widget>(new Widget(...))\n";

        // C++14 写法
        auto p2 = std::make_unique<Widget>(3, 4);
        std::cout << "  C++14 写法: make_unique<Widget>(...)\n";
        std::cout << "  sum = " << p2->sum() << "\n";
    }

    std::cout << "\n--- 7.2 为什么推荐 make_unique ---\n";
    std::cout << "  1. 避免显式 new，符合 RAII 原则\n";
    std::cout << "  2. 异常安全：防止构造函数抛异常时的资源泄漏\n";
    std::cout << "  3. 代码更简洁，无需重复类型名\n";

    std::cout << "\n--- 7.3 make_unique 的变体 ---\n";
    auto arr = std::make_unique<int[]>(5);  // 数组版本
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;
    }
    std::cout << "  make_unique<int[]>(5) 创建的数组: ";
    for (int i = 0; i < 5; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

} // namespace section7_make_unique

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  C++14 特性解析\n";
    std::cout << "  C++11 的完善与小步演进 —— 小版本但提升显著\n";
    std::cout << "============================================================\n";

    section1_generic_lambda::demo();
    section2_return_deduction::demo();
    section3_variable_templates::demo();
    section4_relaxed_constexpr::demo();
    section5_literals_and_attributes::demo();
    section6_capture_initializers::demo();
    section7_make_unique::demo();

    std::cout << "\n============================================================\n";
    std::cout << "  C++14 总结：虽然是小版本，但泛型lambda、返回类型推导、\n";
    std::cout << "  变量模板、广义constexpr等特性大大提升了代码简洁性。\n";
    std::cout << "  很多特性是 C++17 特性的基础。\n";
    std::cout << "============================================================\n";

    return 0;
}
