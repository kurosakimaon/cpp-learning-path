/**
 * @file if_constexpr.cpp
 * @brief C++17 if constexpr 高级应用
 *
 * 知识点覆盖：
 * 1. 编译期分支的基本原理（SFINAE vs if constexpr）
 * 2. 递归模板与 if constexpr（替代模板特化）
 * 3. 类型分发（Type Dispatching）模式
 * 4. SFINAE 友好的编译期检测
 * 5. 元编程中的实际应用：类型计算、编译期算法
 * 6. 与 constexpr lambda 的协同
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic if_constexpr.cpp -o if_constexpr
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <tuple>
#include <array>
#include <numeric>

// ============================================================
// 第一部分：if constexpr 基础与 SFINAE 对比
// ============================================================

namespace basics {

// 1.1 为什么需要 if constexpr？
// 普通的 if 是运行时分支，即使条件在编译期已知，两个分支也必须都能编译通过

// 反面例子：这在 C++14 中会编译失败
// template <typename T>
// auto get_value(T t) {
//     if (std::is_pointer_v<T>) {
//         return *t;  // 当 T 不是指针时，这行无法编译
//     } else {
//         return t;
//     }
// }

// C++14 解决方案：SFINAE + 重载
template <typename T>
auto get_value_impl(T t, std::true_type /*is_pointer*/) {
    return *t;
}

template <typename T>
auto get_value_impl(T t, std::false_type /*is_pointer*/) {
    return t;
}

template <typename T>
auto get_value_cpp14(T t) {
    return get_value_impl(t, std::is_pointer<T>{});
}

// C++17 解决方案：if constexpr
template <typename T>
auto get_value_cpp17(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;  // 只有当 T 是指针时，这个分支才会被实例化
    } else {
        return t;   // 只有当 T 不是指针时，这个分支才会被实例化
    }
}

void demo_basic_comparison() {
    std::cout << "===== 1.1 SFINAE vs if constexpr =====\n";

    int x = 42;
    int* p = &x;

    // C++14 方式
    std::cout << "C++14 SFINAE 方式:\n";
    std::cout << "  get_value(42) = " << get_value_cpp14(42) << "\n";
    std::cout << "  get_value(p)  = " << get_value_cpp14(p) << "\n";

    // C++17 方式
    std::cout << "C++17 if constexpr 方式:\n";
    std::cout << "  get_value(42) = " << get_value_cpp17(42) << "\n";
    std::cout << "  get_value(p)  = " << get_value_cpp17(p) << "\n";
}

// 1.2 if constexpr 的关键特性：丢弃的分支不实例化
// 这意味着丢弃分支中的语法错误也不会导致编译失败（在模板内）

template <typename T>
void maybe_print(const T& value) {
    if constexpr (std::is_same_v<T, int>) {
        // 对于非 int 类型，这个分支被丢弃，不会编译
        std::cout << "整型值: " << value << "\n";
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "字符串值: " << value << "\n";
    } else {
        // 其他类型，不打印
        // 注意：即使这里写了不能编译的代码，只要不被实例化就没事
        // （但在模板外的 if constexpr，两个分支仍然都会被检查语法）
        std::cout << "不支持的类型\n";
    }
}

void demo_discarded_branch() {
    std::cout << "\n===== 1.2 丢弃分支不实例化 =====\n";

    maybe_print(42);
    maybe_print(std::string("hello"));
    // maybe_print(3.14);  // 会走到 else 分支，打印"不支持的类型"
}

} // namespace basics


// ============================================================
// 第二部分：递归模板与 if constexpr
// ============================================================

namespace recursive_template {

// 2.1 经典例子：计算 tuple 中所有数值的和
// C++14 写法：需要模板特化或重载作为终止条件

// C++17 写法：用 if constexpr 终止递归
template <size_t I = 0, typename... Ts>
constexpr auto tuple_sum(const std::tuple<Ts...>& t) {
    if constexpr (I == sizeof...(Ts)) {
        return 0;  // 递归终止：返回 0（整数类型，作为累加起点）
    } else {
        return std::get<I>(t) + tuple_sum<I + 1>(t);
    }
}

void demo_tuple_sum() {
    std::cout << "\n===== 2.1 递归模板 + if constexpr (tuple求和) =====\n";

    constexpr auto t = std::make_tuple(1, 2, 3, 4, 5);
    constexpr auto sum = tuple_sum(t);
    static_assert(sum == 15, "sum should be 15");

    std::cout << "tuple (1,2,3,4,5) 之和 = " << sum << " (编译期计算)\n";

    // 混合类型的 tuple
    auto t2 = std::make_tuple(1, 2.5, 3);
    auto sum2 = tuple_sum(t2);
    std::cout << "tuple (1, 2.5, 3) 之和 = " << sum2
              << " (类型为 " << typeid(sum2).name() << ")\n";
}

// 2.2 编译期计算阶乘
template <int N>
constexpr int factorial() {
    if constexpr (N <= 1) {
        return 1;
    } else {
        return N * factorial<N - 1>();
    }
}

void demo_factorial() {
    std::cout << "\n===== 2.2 编译期阶乘 =====\n";

    static_assert(factorial<0>() == 1);
    static_assert(factorial<1>() == 1);
    static_assert(factorial<5>() == 120);
    static_assert(factorial<10>() == 3628800);

    std::cout << "factorial<5>()  = " << factorial<5>() << "\n";
    std::cout << "factorial<10>() = " << factorial<10>() << "\n";
    std::cout << "所有计算均在编译期完成（static_assert 验证）\n";
}

// 2.3 打印 tuple 的所有元素
template <size_t I = 0, typename... Ts>
void print_tuple(const std::tuple<Ts...>& t) {
    if constexpr (I < sizeof...(Ts)) {
        if constexpr (I > 0) {
            std::cout << ", ";
        }
        std::cout << std::get<I>(t);
        print_tuple<I + 1>(t);
    }
}

void demo_print_tuple() {
    std::cout << "\n===== 2.3 打印 tuple 所有元素 =====\n";

    auto t = std::make_tuple(1, "hello", 3.14, 'x');
    std::cout << "tuple: (";
    print_tuple(t);
    std::cout << ")\n";
}

} // namespace recursive_template


// ============================================================
// 第三部分：类型分发（Type Dispatching）
// ============================================================

namespace type_dispatch {

// 3.1 基于类型特性的不同处理策略
// 场景：一个通用的 serialize 函数，针对不同类型有不同的序列化方式

struct SerializableClass {
    std::string serialize() const { return "SerializedObject"; }
};

// 版本1：支持 serialize() 成员函数的类型
// 版本2：算术类型（int, double 等）
// 版本3：字符串类型
// 版本4：其他类型（报错）

template <typename T>
std::string serialize(const T& obj) {
    if constexpr (std::is_arithmetic_v<T>) {
        // 算术类型：转字符串
        return std::to_string(obj);
    } else if constexpr (std::is_same_v<T, std::string>) {
        // 字符串：直接返回
        return obj;
    } else if constexpr (std::is_same_v<T, const char*>) {
        // C 风格字符串
        return std::string(obj);
    } else if constexpr (requires { obj.serialize(); }) {
        // 有 serialize() 方法的类型（C++20 concepts，这里用 requires 表达式示意）
        // 注意：C++17 中 requires 表达式不是所有编译器都支持，
        // 实际工程中常用检测惯用法（见下文 SFINAE 检测）
        return obj.serialize();
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for serialization");
        return "";
    }
}

// 3.2 C++17 方式：用 void_t 检测成员函数（SFINAE 检测 + if constexpr）

// 检测是否有 serialize() 成员函数
template <typename T, typename = void>
struct has_serialize : std::false_type {};

template <typename T>
struct has_serialize<T, std::void_t<decltype(std::declval<T>().serialize())>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_serialize_v = has_serialize<T>::value;

// 使用检测结果的 serialize 函数（C++17 完全兼容版）
template <typename T>
std::string serialize_v2(const T& obj) {
    if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(obj);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return obj;
    } else if constexpr (has_serialize_v<T>) {
        return obj.serialize();
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for serialize_v2");
        return {};
    }
}

void demo_type_dispatch() {
    std::cout << "\n===== 3. 类型分发模式 =====\n";

    std::cout << "serialize(42) = " << serialize_v2(42) << "\n";
    std::cout << "serialize(3.14) = " << serialize_v2(3.14) << "\n";
    std::cout << "serialize(string) = " << serialize_v2(std::string("hello")) << "\n";

    SerializableClass sc;
    std::cout << "serialize(SerializableClass) = " << serialize_v2(sc) << "\n";
}

// 3.3 迭代器类型分发：输入迭代器 vs 随机访问迭代器
template <typename Iter>
auto advance_iter(Iter it, int n) {
    if constexpr (std::is_base_of_v<
        std::random_access_iterator_tag,
        typename std::iterator_traits<Iter>::iterator_category
    >) {
        // 随机访问迭代器：直接 O(1) 跳转
        it += n;
        std::cout << "使用随机访问迭代器 (O(1))\n";
    } else {
        // 其他迭代器：逐步前进
        if (n >= 0) {
            for (int i = 0; i < n; ++i) ++it;
        } else {
            for (int i = 0; i < -n; ++i) --it;
        }
        std::cout << "使用逐步迭代 (O(n))\n";
    }
    return it;
}

void demo_iterator_dispatch() {
    std::cout << "\n===== 3.3 迭代器类型分发 =====\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto it1 = advance_iter(vec.begin(), 2);
    std::cout << "vector 迭代器前进 2 步后的值: " << *it1 << "\n";

    std::list<int> lst = {10, 20, 30, 40, 50};
    auto it2 = advance_iter(lst.begin(), 3);
    std::cout << "list 迭代器前进 3 步后的值: " << *it2 << "\n";
}

} // namespace type_dispatch


// ============================================================
// 第四部分：SFINAE 友好的编译期检测模式
// ============================================================

namespace sfinae_detection {

// 4.1 检测成员类型
template <typename T, typename = void>
struct has_value_type : std::false_type {};

template <typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_value_type_v = has_value_type<T>::value;

// 4.2 检测成员函数的返回类型
template <typename T, typename = void>
struct has_size_method : std::false_type {};

template <typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_size_method_v = has_size_method<T>::value;

// 4.3 检测操作符（如 operator[]）
template <typename T, typename = void>
struct has_subscript : std::false_type {};

template <typename T>
struct has_subscript<T, std::void_t<decltype(std::declval<T>()[0])>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_subscript_v = has_subscript<T>::value;

// 4.4 泛型打印函数：智能选择打印方式
template <typename T>
void smart_print(const T& container) {
    if constexpr (has_subscript_v<T> && has_size_method_v<T>) {
        // 支持下标和 size 的容器（如 vector, array）
        std::cout << "[";
        for (size_t i = 0; i < container.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << container[i];
        }
        std::cout << "]";
    } else if constexpr (has_size_method_v<T>) {
        // 有 size 但不支持下标（如 list）
        std::cout << "{";
        bool first = true;
        for (const auto& elem : container) {
            if (!first) std::cout << ", ";
            first = false;
            std::cout << elem;
        }
        std::cout << "}";
    } else {
        // 普通值
        std::cout << container;
    }
    std::cout << "\n";
}

void demo_sfinae_detection() {
    std::cout << "\n===== 4. SFINAE 编译期检测 =====\n";

    std::cout << "int 有 value_type 吗？ " << std::boolalpha
              << has_value_type_v<int> << "\n";
    std::cout << "vector<int> 有 value_type 吗？ "
              << has_value_type_v<std::vector<int>> << "\n";

    std::cout << "\n智能打印:\n";
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::cout << "  vector: ";
    smart_print(v);

    std::list<int> l = {10, 20, 30};
    std::cout << "  list: ";
    smart_print(l);

    std::cout << "  int: ";
    smart_print(42);
}

} // namespace sfinae_detection


// ============================================================
// 第五部分：元编程高级应用
// ============================================================

namespace metaprogramming {

// 5.1 编译期类型列表处理
template <typename... Ts>
struct TypeList {
    static constexpr size_t size = sizeof...(Ts);
};

// 获取类型列表中的第 N 个类型
template <size_t N, typename... Ts>
struct nth_type;

template <size_t N, typename Head, typename... Tail>
struct nth_type<N, Head, Tail...> {
    static_assert(N < sizeof...(Tail) + 1, "Index out of bounds");
    using type = typename nth_type<N - 1, Tail...>::type;
};

template <typename Head, typename... Tail>
struct nth_type<0, Head, Tail...> {
    using type = Head;
};

template <size_t N, typename... Ts>
using nth_type_t = typename nth_type<N, Ts...>::type;

// 用 if constexpr 实现编译期类型过滤（需要配合其他技术）
// 这里演示一个更实用的：根据条件选择类型

template <bool Condition, typename T, typename F>
struct conditional_type {
    using type = F;
};

template <typename T, typename F>
struct conditional_type<true, T, F> {
    using type = T;
};

template <bool Condition, typename T, typename F>
using conditional_type_t = typename conditional_type<Condition, T, F>::type;

// 5.2 编译期算法：查找类型列表中满足条件的第一个类型
// 这里用 if constexpr + 递归来实现

// 辅助：判断类型是否满足条件（示例：是否为整数类型）
template <typename T>
inline constexpr bool is_integral_v = std::is_integral_v<T>;

// 5.3 constexpr 函数与 if constexpr 的结合
constexpr int gcd(int a, int b) {
    if constexpr (true) {  // constexpr 函数中也可以用 if constexpr（但没必要）
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
}

// 更有意义的：constexpr 编译期数组算法
template <size_t N>
constexpr int array_sum(const std::array<int, N>& arr) {
    int sum = 0;
    for (size_t i = 0; i < N; ++i) {
        sum += arr[i];
    }
    return sum;
}

void demo_metaprogramming() {
    std::cout << "\n===== 5. 元编程应用 =====\n";

    // 类型列表
    using MyTypes = TypeList<int, double, char, std::string>;
    std::cout << "TypeList 大小: " << MyTypes::size << "\n";

    // 第 N 个类型
    static_assert(std::is_same_v<nth_type_t<0, int, double, char>, int>);
    static_assert(std::is_same_v<nth_type_t<2, int, double, char>, char>);
    std::cout << "nth_type_t<0, int, double, char> = int\n";
    std::cout << "nth_type_t<2, int, double, char> = char\n";

    // 条件类型选择
    static_assert(std::is_same_v<conditional_type_t<true, int, double>, int>);
    static_assert(std::is_same_v<conditional_type_t<false, int, double>, double>);
    std::cout << "conditional_type_t<true, int, double> = int\n";

    // 编译期 gcd
    static_assert(gcd(12, 18) == 6);
    static_assert(gcd(7, 13) == 1);
    std::cout << "gcd(12, 18) = " << gcd(12, 18) << " (编译期计算)\n";

    // 编译期数组求和
    constexpr std::array<int, 5> arr = {1, 2, 3, 4, 5};
    static_assert(array_sum(arr) == 15);
    std::cout << "array_sum({1,2,3,4,5}) = " << array_sum(arr) << " (编译期)\n";
}

} // namespace metaprogramming


// ============================================================
// 第六部分：constexpr lambda 与高级模式
// ============================================================

namespace constexpr_lambda {

// 6.1 C++17 的 constexpr lambda
// C++17 起，lambda 可以声明为 constexpr，即在编译期调用

constexpr auto factorial_lambda = [](int n) {
    if (n <= 1) return 1;
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
};

// 6.2 泛型 lambda + if constexpr
constexpr auto process = [](auto value) {
    using T = decltype(value);
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return value * 1.5;
    } else {
        return value;  // 其他类型原样返回
    }
};

void demo_constexpr_lambda() {
    std::cout << "\n===== 6. constexpr lambda =====\n";

    // 编译期调用
    static_assert(factorial_lambda(5) == 120);
    std::cout << "factorial_lambda(5) = " << factorial_lambda(5)
              << " (编译期计算)\n";

    // 泛型 lambda + if constexpr
    std::cout << "process(10) = " << process(10) << " (int, x2)\n";
    std::cout << "process(10.0) = " << process(10.0) << " (double, x1.5)\n";
    std::cout << "process(string) = " << process(std::string("hello")) << "\n";
}

// 6.3 编译期查找算法（用 constexpr lambda 作为谓词）
template <typename T, size_t N, typename Pred>
constexpr const T* find_if(const std::array<T, N>& arr, Pred pred) {
    for (size_t i = 0; i < N; ++i) {
        if (pred(arr[i])) {
            return &arr[i];
        }
    }
    return nullptr;
}

void demo_find_if() {
    std::cout << "\n===== 6.3 编译期查找算法 =====\n";

    constexpr std::array<int, 6> numbers = {2, 4, 6, 7, 8, 10};

    // 查找第一个奇数
    constexpr auto is_odd = [](int x) { return x % 2 != 0; };
    constexpr const int* first_odd = find_if(numbers, is_odd);
    static_assert(first_odd != nullptr && *first_odd == 7);

    // 查找第一个大于 9 的数
    constexpr auto greater_than_9 = [](int x) { return x > 9; };
    constexpr const int* gt9 = find_if(numbers, greater_than_9);
    static_assert(gt9 != nullptr && *gt9 == 10);

    std::cout << "第一个奇数: " << *first_odd << "\n";
    std::cout << "第一个 >9 的数: " << *gt9 << "\n";
    std::cout << "查找在编译期完成\n";
}

} // namespace constexpr_lambda


// ============================================================
// 第七部分：注意事项与陷阱
// ============================================================

namespace pitfalls {

// 7.1 陷阱：if constexpr 外的分支仍然会被检查
// 只有在模板内部，且条件依赖于模板参数时，丢弃的分支才不会被实例化
// 非模板代码中的 if constexpr，两个分支都会被完整编译

void demo_non_template_pitfall() {
    std::cout << "\n===== 7.1 非模板中的 if constexpr =====\n";

    // 这是普通函数（非模板），if constexpr 的两个分支都会被检查
    if constexpr (true) {
        std::cout << "这个分支会执行\n";
    } else {
        // 即使这个分支永远不会执行，它也必须语法正确且可编译
        // int x = "hello";  // 错误！即使在 false 分支也会报错
        std::cout << "这个分支被丢弃，但仍然会被编译检查\n";
    }
    std::cout << "非模板中 if constexpr 的两个分支都会被语法检查\n";
}

// 7.2 陷阱：return 类型不一致
// if constexpr 的不同分支返回不同类型时，函数的返回类型会根据所有分支推导

template <typename T>
auto inconsistent_return(T x) {
    if constexpr (std::is_integral_v<T>) {
        return 1;  // int
    } else {
        return 1.5;  // double
    }
    // 没问题！因为每次实例化只有一个分支存在，返回类型是确定的
}

void demo_return_types() {
    std::cout << "\n===== 7.2 不同分支返回不同类型 =====\n";

    auto r1 = inconsistent_return(42);   // 返回 int
    auto r2 = inconsistent_return(3.14); // 返回 double

    static_assert(std::is_same_v<decltype(r1), int>);
    static_assert(std::is_same_v<decltype(r2), double>);

    std::cout << "inconsistent_return(int) 返回 int\n";
    std::cout << "inconsistent_return(double) 返回 double\n";
}

// 7.3 陷阱：不要在 if constexpr 中使用 runtime 变量
// if constexpr 的条件必须是编译期常量表达式

void demo_constexpr_condition() {
    std::cout << "\n===== 7.3 条件必须是编译期常量 =====\n";

    constexpr int N = 5;
    if constexpr (N > 3) {
        std::cout << "N > 3 (编译期判断)\n";
    }

    int runtime_n = 5;
    // if constexpr (runtime_n > 3) {  // 错误！runtime_n 不是编译期常量
    //     ...
    // }
    std::cout << "if constexpr 的条件必须是编译期常量表达式\n";
}

} // namespace pitfalls


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 if constexpr 高级应用\n";
    std::cout << "============================================\n";

    basics::demo_basic_comparison();
    basics::demo_discarded_branch();

    recursive_template::demo_tuple_sum();
    recursive_template::demo_factorial();
    recursive_template::demo_print_tuple();

    type_dispatch::demo_type_dispatch();
    type_dispatch::demo_iterator_dispatch();

    sfinae_detection::demo_sfinae_detection();

    metaprogramming::demo_metaprogramming();

    constexpr_lambda::demo_constexpr_lambda();
    constexpr_lambda::demo_find_if();

    pitfalls::demo_non_template_pitfall();
    pitfalls::demo_return_types();
    pitfalls::demo_constexpr_condition();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
