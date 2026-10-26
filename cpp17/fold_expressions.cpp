/**
 * @file fold_expressions.cpp
 * @brief C++17 折叠表达式（Fold Expressions）深度解析
 *
 * 知识点覆盖：
 * 1. 一元右折叠、一元左折叠的语法与结合性
 * 2. 二元折叠及其初始值语义
 * 3. 运算符折叠：算术、逻辑、逗号、位运算等
 * 4. 结合可变参数模板的实际应用：求和、打印、all_of/any_of
 * 5. 高级模式：递归展开、类型计算、参数包过滤
 * 6. 折叠表达式与完美转发的协同
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic fold_expressions.cpp -o fold_expressions
 */

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <type_traits>
#include <utility>
#include <sstream>
#include <numeric>
#include <algorithm>

// ============================================================
// 第一部分：折叠表达式基础语法
// ============================================================

namespace basics {

// 折叠表达式有四种形式：
// 1. 一元右折叠 (pack op ...)      -> e1 op (e2 op (... op eN))
// 2. 一元左折叠 (... op pack)      -> (((e1 op e2) op ...) op eN)
// 3. 二元右折叠 (pack op ... op init) -> e1 op (e2 op (... op (eN op init)))
// 4. 二元左折叠 (init op ... op pack) -> (((init op e1) op e2) op ...) op eN

// 1.1 一元右折叠：求和
template <typename... Args>
auto sum_right(Args... args) {
    return (args + ...);  // 一元右折叠：e1 + (e2 + (e3 + ...))
    // 等价于 args[0] + (args[1] + (args[2] + ... ))
}

// 1.2 一元左折叠：求和
template <typename... Args>
auto sum_left(Args... args) {
    return (... + args);  // 一元左折叠：(((e1 + e2) + e3) + ...) + eN
    // 等价于 (((args[0] + args[1]) + args[2]) + ... )
}

// 对于加法（满足结合律），左折叠和右折叠结果相同
// 但对于不满足结合律的运算符（如减法），结果不同

template <typename... Args>
auto sub_right(Args... args) {
    return (args - ...);  // 右折叠：e1 - (e2 - (e3 - e4))
}

template <typename... Args>
auto sub_left(Args... args) {
    return (... - args);  // 左折叠：((e1 - e2) - e3) - e4
}

void demo_basic_fold() {
    std::cout << "===== 1. 折叠表达式基础 =====\n";

    // 加法：左右折叠结果相同
    std::cout << "sum_right(1,2,3,4,5) = " << sum_right(1,2,3,4,5) << "\n";
    std::cout << "sum_left(1,2,3,4,5)  = " << sum_left(1,2,3,4,5) << "\n";

    // 减法：左右折叠结果不同
    std::cout << "\n减法：结合性不同导致结果不同\n";
    std::cout << "sub_right(10, 3, 2, 1) = " << sub_right(10, 3, 2, 1)
              << "  (10 - (3 - (2 - 1))) = 10 - (3 - 1) = 10 - 2 = 8\n";
    std::cout << "sub_left(10, 3, 2, 1)  = " << sub_left(10, 3, 2, 1)
              << "  (((10 - 3) - 2) - 1) = (7 - 2) - 1 = 4\n";
}

// 1.3 二元折叠：带初始值
// 当参数包为空时，一元折叠对大多数运算符会产生编译错误
// 二元折叠在参数包为空时返回初始值

template <typename... Args>
auto sum_with_init(Args... args) {
    return (args + ... + 0);  // 二元右折叠，初始值 0
}

template <typename... Args>
auto product_with_init(Args... args) {
    return (1 * ... * args);  // 二元左折叠，初始值 1
}

void demo_binary_fold() {
    std::cout << "\n===== 1.3 二元折叠（带初始值） =====\n";

    std::cout << "sum_with_init(1,2,3) = " << sum_with_init(1,2,3) << "\n";
    std::cout << "sum_with_init()      = " << sum_with_init()
              << "  (空包时返回初始值 0)\n";

    std::cout << "product_with_init(2,3,4) = " << product_with_init(2,3,4) << "\n";
    std::cout << "product_with_init()      = " << product_with_init()
              << "  (空包时返回初始值 1)\n";
}

// 1.4 空参数包的一元折叠
// 以下运算符在空包时有特殊处理（可以编译）：
// - && -> true
// - || -> false
// - ,  -> void()

template <typename... Args>
bool all_true(Args... args) {
    return (... && args);  // 空包返回 true
}

template <typename... Args>
bool any_true(Args... args) {
    return (... || args);  // 空包返回 false
}

void demo_empty_pack() {
    std::cout << "\n===== 1.4 空参数包特殊情况 =====\n";

    std::cout << std::boolalpha;
    std::cout << "all_true(true, true, true) = " << all_true(true, true, true) << "\n";
    std::cout << "all_true(true, false, true) = " << all_true(true, false, true) << "\n";
    std::cout << "all_true() (空包) = " << all_true()
              << "  (逻辑与的空包恒为 true)\n";

    std::cout << "any_true(false, false, true) = " << any_true(false, false, true) << "\n";
    std::cout << "any_true() (空包) = " << any_true()
              << "  (逻辑或的空包恒为 false)\n";
    std::cout << std::noboolalpha;
}

} // namespace basics


// ============================================================
// 第二部分：各种运算符的折叠应用
// ============================================================

namespace various_ops {

// 2.1 逗号运算符折叠：对每个参数执行操作
// 这是最常用的折叠模式之一

template <typename... Args>
void print_all(Args&&... args) {
    ((std::cout << args << " "), ...);  // 逗号运算符左折叠
    // 等价于: ((cout << a1 << " ") << a2 << " ") ... 不对
    // 实际是: (cout << a1 << " "), (cout << a2 << " "), ...
    // 逗号运算符依次执行每个表达式
    std::cout << "\n";
}

// 2.2 在每个元素上调用成员函数
template <typename Container, typename... Args>
void push_back_all(Container& c, Args&&... args) {
    (c.push_back(std::forward<Args>(args)), ...);
    // 逗号折叠：依次执行每个 push_back
}

void demo_comma_fold() {
    std::cout << "\n===== 2.1 逗号运算符折叠 =====\n";

    print_all(1, 2.5, "hello", 'x');

    std::vector<int> vec;
    push_back_all(vec, 1, 2, 3, 4, 5);
    std::cout << "vector 大小: " << vec.size() << "\n";
    std::cout << "元素: ";
    for (int v : vec) std::cout << v << " ";
    std::cout << "\n";
}

// 2.3 位运算折叠
template <typename... Args>
auto bit_or_all(Args... args) {
    return (args | ...);
}

template <typename... Args>
auto bit_and_all(Args... args) {
    return (args & ...);
}

// 2.4 比较运算的高级应用
// 注意：比较运算符的折叠需要谨慎考虑结合性

// 检查所有值是否相等
template <typename T, typename... Args>
bool all_equal(T first, Args... rest) {
    return ((first == rest) && ...);
    // 展开: (first == rest1) && (first == rest2) && ...
}

// 检查范围：min < x < max 的展开
// 注意：a < b < c 这种写法在 C++ 中是合法的但语义不对（a<b 返回 bool，再与 c 比较）
// 折叠表达式可以更安全地表达链式比较

template <typename T, typename... Args>
bool all_greater_than(T threshold, Args... args) {
    return ((args > threshold) && ...);
}

void demo_various_ops() {
    std::cout << "\n===== 2.2 各种运算符折叠 =====\n";

    // 位运算
    std::cout << "bit_or_all(1, 2, 4, 8) = " << bit_or_all(1, 2, 4, 8)
              << " (二进制: 1111 = 15)\n";
    std::cout << "bit_and_all(7, 3, 5) = " << bit_and_all(7, 3, 5)
              << " (7&3=3, 3&5=1)\n";

    // 比较
    std::cout << std::boolalpha;
    std::cout << "all_equal(5, 5, 5, 5) = " << all_equal(5, 5, 5, 5) << "\n";
    std::cout << "all_equal(5, 5, 3, 5) = " << all_equal(5, 5, 3, 5) << "\n";
    std::cout << "all_greater_than(10, 20, 30, 15) = "
              << all_greater_than(10, 20, 30, 15) << "\n";
    std::cout << "all_greater_than(10, 5, 20, 15) = "
              << all_greater_than(10, 5, 20, 15) << "\n";
    std::cout << std::noboolalpha;
}

// 2.5 字符串拼接折叠
template <typename... Args>
std::string concat_strings(Args&&... args) {
    std::string result;
    (result += ... += std::forward<Args>(args));
    // 左折叠：(((result += a1) += a2) += a3) += ...
    return result;
}

void demo_string_concat() {
    std::cout << "\n===== 2.3 字符串拼接 =====\n";

    auto result = concat_strings(std::string("Hello"), std::string(", "),
                                 std::string("World"), std::string("!"));
    std::cout << "拼接结果: " << result << "\n";
}

} // namespace various_ops


// ============================================================
// 第三部分：结合完美转发的高级模式
// ============================================================

namespace perfect_forward {

// 3.1 完美转发 + 折叠：对每个参数应用同一个函数

template <typename Func, typename... Args>
void for_each_arg(Func&& f, Args&&... args) {
    (f(std::forward<Args>(args)), ...);
    // 逗号折叠：依次对每个参数调用 f
}

void demo_for_each_arg() {
    std::cout << "\n===== 3.1 for_each_arg (对每个参数调用函数) =====\n";

    int sum = 0;
    for_each_arg([&sum](int x) { sum += x; std::cout << x << " "; },
                 1, 2, 3, 4, 5);
    std::cout << "\n累加和: " << sum << "\n";

    // 打印不同类型
    std::cout << "打印不同类型: ";
    for_each_arg([](const auto& x) { std::cout << x << "|"; },
                 1, 2.5, "hello", 'x');
    std::cout << "\n";
}

// 3.2 通用的 tuple 遍历（用折叠表达式 + 索引序列）
template <typename Tuple, typename Func, size_t... Is>
void for_each_in_tuple_impl(Tuple&& t, Func&& f, std::index_sequence<Is...>) {
    (f(std::get<Is>(std::forward<Tuple>(t))), ...);
}

template <typename Tuple, typename Func>
void for_each_in_tuple(Tuple&& t, Func&& f) {
    constexpr size_t N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
    for_each_in_tuple_impl(std::forward<Tuple>(t), std::forward<Func>(f),
                           std::make_index_sequence<N>{});
}

void demo_tuple_for_each() {
    std::cout << "\n===== 3.2 tuple 遍历 =====\n";

    auto t = std::make_tuple(1, 2.5, std::string("hello"), 'x');

    std::cout << "tuple 元素: ";
    for_each_in_tuple(t, [](const auto& elem) {
        std::cout << elem << " ";
    });
    std::cout << "\n";

    // 计算 tuple 中数值的和（只算数值类型）
    double total = 0;
    for_each_in_tuple(t, [&total](const auto& elem) {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(elem)>>) {
            total += elem;
        }
    });
    std::cout << "tuple 中数值之和: " << total << "\n";
}

// 3.3 参数包转换：将每个参数转换后收集到 vector 中
template <typename T, typename... Args>
std::vector<T> transform_to_vector(Args&&... args) {
    std::vector<T> result;
    result.reserve(sizeof...(Args));
    (result.push_back(static_cast<T>(std::forward<Args>(args))), ...);
    return result;
}

void demo_transform_to_vector() {
    std::cout << "\n===== 3.3 参数包转 vector =====\n";

    auto vec = transform_to_vector<double>(1, 2, 3.5f, 4.0);
    std::cout << "转换为 double 类型的 vector: ";
    for (double v : vec) std::cout << v << " ";
    std::cout << "\n";
}

} // namespace perfect_forward


// ============================================================
// 第四部分：类型计算与编译期折叠
// ============================================================

namespace type_computation {

// 4.1 编译期判断所有类型是否相同
template <typename T, typename... Rest>
inline constexpr bool all_same_v = std::conjunction_v<std::is_same<T, Rest>...>;
// std::conjunction 本质上也是用折叠实现的

// 手动实现版本
template <typename T, typename... Rest>
struct all_same_manual {
    static constexpr bool value = (std::is_same_v<T, Rest> && ...);
};

template <typename... Ts>
inline constexpr bool all_same_manual_v = all_same_manual<Ts...>::value;

// 4.2 计算类型列表的大小之和（每个类型的 sizeof 之和）
template <typename... Ts>
inline constexpr size_t sum_of_sizes = (sizeof(Ts) + ... + 0);

// 4.3 查找最大类型
template <typename T>
struct type_size_wrapper {
    using type = T;
    static constexpr size_t size = sizeof(T);
};

// 4.4 编译期计算：所有参数的最大值

// 递归写法：一次取一个参数，思路最直观
template <typename T>
constexpr T max_val(T value) {
    return value;
}

template <typename T, typename... Rest>
constexpr T max_val(T first, Rest... rest) {
    const T tail = max_val(rest...);
    return first > tail ? first : tail;
}

// 折叠写法：逗号折叠 + 赋值，逐个比较并更新 result
// 注意：折叠表达式只能用运算符，不能直接折叠任意函数调用
template <typename T, typename... Rest>
constexpr T max_fold(T first, Rest... rest) {
    T result = first;
    ((result = rest > result ? rest : result), ...);
    return result;
}

// 另一种实用写法：std::max + initializer_list
// 要求所有参数类型相同（或能推导出共同的 T）
template <typename T, typename... Rest>
constexpr T max_initializer_list(T first, Rest... rest) {
    return std::max({first, rest...});
}

void demo_type_computation() {
    std::cout << "\n===== 4. 类型计算与编译期折叠 =====\n";

    // 类型相同检测
    static_assert(all_same_manual_v<int, int, int>);
    static_assert(!all_same_manual_v<int, double, int>);
    std::cout << std::boolalpha;
    std::cout << "all_same_v<int, int, int> = " << all_same_manual_v<int, int, int> << "\n";
    std::cout << "all_same_v<int, double, int> = " << all_same_manual_v<int, double, int> << "\n";

    // 类型大小之和
    std::cout << "sum_of_sizes<char, short, int, double> = "
              << sum_of_sizes<char, short, int, double>
              << " (1 + 2 + 4 + 8 = 15)\n";

    // 编译期最大值
    static_assert(max_fold(1, 5, 3, 9, 2) == 9);
    std::cout << "max_fold(1, 5, 3, 9, 2) = " << max_fold(1, 5, 3, 9, 2)
              << " (编译期计算)\n";
    std::cout << std::noboolalpha;
}

} // namespace type_computation


// ============================================================
// 第五部分：高级应用模式
// ============================================================

namespace advanced_patterns {

// 5.1 参数包过滤（结合 if constexpr）
// 过滤出满足条件的参数，收集到容器中

template <typename Container, typename Pred, typename... Args>
void filter_into(Container& c, Pred pred, Args&&... args) {
    auto try_push = [&](auto&& arg) {
        if constexpr (std::is_invocable_r_v<bool, Pred, decltype(arg)>) {
            if (pred(std::forward<decltype(arg)>(arg))) {
                c.push_back(std::forward<decltype(arg)>(arg));
            }
        }
    };
    (try_push(std::forward<Args>(args)), ...);
}

void demo_filter() {
    std::cout << "\n===== 5.1 参数包过滤 =====\n";

    std::vector<int> evens;
    auto is_even = [](int x) { return x % 2 == 0; };
    filter_into(evens, is_even, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    std::cout << "过滤偶数: ";
    for (int v : evens) std::cout << v << " ";
    std::cout << "\n";
}

// 5.2 安全的多继承构造函数转发（使用折叠表达式 + 完美转发）
// 这是一个高级模式：可变参数基类的构造

template <typename... Bases>
struct MultiInherit : Bases... {
    // 完美转发构造每个基类
    template <typename... Args>
    MultiInherit(Args&&... args) : Bases(std::forward<Args>(args))... {
        // 注意：这不是折叠表达式，而是参数包展开
        // 但它展示了可变参数在构造中的威力
    }
};

struct BaseA {
    int a;
    BaseA(int x) : a(x) {}
};

struct BaseB {
    double b;
    BaseB(double y) : b(y) {}
};

struct BaseC {
    std::string c;
    BaseC(std::string s) : c(std::move(s)) {}
};

void demo_multi_inherit() {
    std::cout << "\n===== 5.2 多继承构造转发 =====\n";

    MultiInherit<BaseA, BaseB, BaseC> obj(42, 3.14, "hello");
    std::cout << "BaseA::a = " << obj.a << "\n";
    std::cout << "BaseB::b = " << obj.b << "\n";
    std::cout << "BaseC::c = " << obj.c << "\n";
}

// 5.3 编译期字符串拼接（进阶）
// 使用 constexpr + 折叠表达式在编译期拼接字符串字面量

// 简单的编译期字符串
template <size_t N>
struct FixedString {
    char data[N]{};
    static constexpr size_t size = N - 1;  // 不含 null 终止符

    constexpr FixedString() = default;

    constexpr FixedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) data[i] = str[i];
    }
};

// 两个 FixedString 相加，结果仍然是编译期字符串
template <size_t N, size_t M>
constexpr auto operator+(const FixedString<N>& a, const FixedString<M>& b) {
    FixedString<N + M - 1> result{};  // 两段长度相加，只保留一个 null
    for (size_t i = 0; i < N - 1; ++i) result.data[i] = a.data[i];
    for (size_t i = 0; i < M; ++i) result.data[N - 1 + i] = b.data[i];
    return result;
}

// 用折叠表达式拼接任意多个编译期字符串
template <typename... Strings>
constexpr auto concat(Strings... strs) {
    static_assert(sizeof...(Strings) > 0, "至少需要一个字符串");
    return (strs + ...);
}

void demo_constexpr_concat() {
    std::cout << "\n===== 5.3 编译期字符串拼接 =====\n";

    constexpr auto greeting = concat(
        FixedString("Hello"), FixedString(", "), FixedString("World!"));
    static_assert(greeting.size == 13);
    std::cout << "拼接结果: " << greeting.data << "\n";
    std::cout << "长度: " << greeting.size << "（编译期确定）\n";
}

// 5.4 实际工程模式：统一的参数校验
template <typename... Args>
bool all_positive(Args... args) {
    return ((args > 0) && ...);
}

template <typename... Args>
bool any_negative(Args... args) {
    return ((args < 0) || ...);
}

void demo_validation() {
    std::cout << "\n===== 5.3 参数校验模式 =====\n";

    std::cout << std::boolalpha;
    std::cout << "all_positive(1, 2, 3) = " << all_positive(1, 2, 3) << "\n";
    std::cout << "all_positive(1, -2, 3) = " << all_positive(1, -2, 3) << "\n";
    std::cout << "any_negative(1, 2, 3) = " << any_negative(1, 2, 3) << "\n";
    std::cout << "any_negative(1, -2, 3) = " << any_negative(1, -2, 3) << "\n";
    std::cout << std::noboolalpha;
}

} // namespace advanced_patterns


// ============================================================
// 第六部分：注意事项与陷阱
// ============================================================

namespace pitfalls {

// 6.1 陷阱：运算符优先级与结合性
// 折叠表达式的结合性非常重要，特别是对于不满足结合律的运算符

// 6.2 陷阱：重载运算符与折叠
// 自定义类型的重载运算符也可以参与折叠

struct IntWrapper {
    int value;
    IntWrapper(int v) : value(v) {}

    IntWrapper operator+(const IntWrapper& other) const {
        return IntWrapper(value + other.value);
    }
};

template <typename... Args>
auto sum_wrappers(Args... args) {
    return (args + ...);
}

void demo_custom_operator_fold() {
    std::cout << "\n===== 6.1 自定义类型运算符折叠 =====\n";

    IntWrapper result = sum_wrappers(IntWrapper(1), IntWrapper(2), IntWrapper(3));
    std::cout << "自定义类型折叠求和: " << result.value << "\n";
}

// 6.3 陷阱：空包 + 无默认值的运算符
// 一元折叠的空包只对 && || , 三个运算符有默认值
// 其他运算符的一元折叠在空包时是 ill-formed

template <typename... Args>
auto safe_sum(Args... args) {
    // 用二元折叠，提供初始值，空包时安全
    return (args + ... + 0);
}

// 6.4 陷阱：折叠表达式中函数调用的求值顺序
// 逗号运算符的折叠有确定的求值顺序（从左到右）
// 但其他运算符不一定（函数参数的求值顺序是未指定的，不是未定义行为）

template <typename... Args>
void print_with_index(Args... args) {
    int idx = 0;
    // 逗号折叠保证从左到右执行
    ((std::cout << "[" << idx++ << "]=" << args << " "), ...);
    std::cout << "\n";
}

void demo_eval_order() {
    std::cout << "\n===== 6.2 求值顺序（逗号折叠保证顺序） =====\n";
    print_with_index("a", "b", "c", "d");
}

// 6.5 陷阱：不要过度使用折叠表达式
// 复杂的折叠表达式可读性差，适当使用初始化列表或递归更清晰

void demo_readability() {
    std::cout << "\n===== 6.3 可读性建议 =====\n";
    std::cout << "简单的求和/打印/逻辑运算：适合折叠表达式\n";
    std::cout << "复杂的多步逻辑：考虑用初始化列表 + 普通循环\n";
    std::cout << "递归终止条件明确的：用 if constexpr + 递归\n";
}

} // namespace pitfalls


// ============================================================
// 第七部分：综合示例 - 泛型日志库
// ============================================================

namespace log_library_example {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

const char* level_name(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
    }
    return "UNKNOWN";
}

// 泛型日志函数：接受任意数量、任意类型的参数
template <typename... Args>
void log(LogLevel level, Args&&... args) {
    std::cout << "[" << level_name(level) << "] ";

    // 用逗号折叠打印所有参数
    (std::cout << ... << std::forward<Args>(args));
    // 左折叠：(((cout << a1) << a2) << a3) << ...

    std::cout << "\n";
}

// 条件日志：满足条件才输出
template <typename... Args>
void log_if(bool condition, LogLevel level, Args&&... args) {
    if (condition) {
        log(level, std::forward<Args>(args)...);
    }
}

// 带前缀的日志
template <typename... Args>
void log_with_prefix(const std::string& prefix, LogLevel level, Args&&... args) {
    std::cout << "[" << level_name(level) << "] [" << prefix << "] ";
    (std::cout << ... << std::forward<Args>(args));
    std::cout << "\n";
}

void demo_log_library() {
    std::cout << "\n===== 7. 综合示例：泛型日志库 =====\n";

    log(LogLevel::INFO, "服务器启动，端口: ", 8080, ", PID: ", 12345);
    log(LogLevel::DEBUG, "调试信息: x=", 10, ", y=", 20);
    log(LogLevel::WARNING, "警告: 内存使用超过 ", 80, "%");
    log(LogLevel::ERROR, "错误: 连接失败，错误码: ", -1);

    log_if(true, LogLevel::INFO, "条件日志: 这条会显示");
    log_if(false, LogLevel::INFO, "条件日志: 这条不会显示");

    log_with_prefix("Network", LogLevel::INFO, "收到连接，IP: ", "192.168.1.1");
    log_with_prefix("Database", LogLevel::ERROR, "查询超时，SQL: ", "SELECT * FROM users");
}

} // namespace log_library_example


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 折叠表达式（Fold Expressions）深度解析\n";
    std::cout << "============================================\n";

    basics::demo_basic_fold();
    basics::demo_binary_fold();
    basics::demo_empty_pack();

    various_ops::demo_comma_fold();
    various_ops::demo_various_ops();
    various_ops::demo_string_concat();

    perfect_forward::demo_for_each_arg();
    perfect_forward::demo_tuple_for_each();
    perfect_forward::demo_transform_to_vector();

    type_computation::demo_type_computation();

    advanced_patterns::demo_filter();
    advanced_patterns::demo_multi_inherit();
    advanced_patterns::demo_constexpr_concat();
    advanced_patterns::demo_validation();

    pitfalls::demo_custom_operator_fold();
    pitfalls::demo_eval_order();
    pitfalls::demo_readability();

    log_library_example::demo_log_library();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
