/**
 * @file evolution_comparison.cpp
 * @brief C++11 → C++14 → C++17 演进对比
 *
 * 本文件通过同一功能在不同版本 C++ 中的不同实现，
 * 直观展示现代 C++ 的演进脉络。
 *
 * 每个主题都展示了从 C++11 到 C++17 的写法变化，
 * 帮助你理解每个版本解决了什么问题、带来了什么改进。
 *
 * 覆盖主题：
 * 1. 遍历与访问（range-for → 结构化绑定）
 * 2. 类型推导（尾置返回 → auto → if constexpr）
 * 3. 多态变体（继承虚函数 → variant + visit）
 * 4. 可选值（指针/nullptr → optional）
 * 5. 求和算法（递归变参 → 折叠表达式）
 * 6. 元组访问（std::get → 结构化绑定）
 * 7. 模板元编程（SFINAE → void_t → if constexpr）
 * 8. 函数式编程（bind → lambda → 泛型lambda）
 * 9. 字符串处理（string → string_view）
 * 10. 错误处理（返回码 → 异常 → expected/optional 模式）
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic evolution_comparison.cpp -o evo_compare
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <type_traits>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <any>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <chrono>
#include <iomanip>

// 辅助：打印分隔
void section(const char* title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

// ============================================================================
// 1. 遍历与访问容器
// ============================================================================
namespace topic1_iteration {

// 场景：遍历一个 map，同时访问 key 和 value

void cpp98_style(const std::map<std::string, int>& m) {
    std::cout << "  [C++98] 迭代器遍历：\n";
    for (std::map<std::string, int>::const_iterator it = m.begin();
         it != m.end(); ++it) {
        std::cout << "    " << it->first << " -> " << it->second << "\n";
    }
}

void cpp11_style(const std::map<std::string, int>& m) {
    std::cout << "  [C++11] 范围 for + auto + pair::first/second：\n";
    for (const auto& pair : m) {
        std::cout << "    " << pair.first << " -> " << pair.second << "\n";
    }
}

void cpp17_style(const std::map<std::string, int>& m) {
    std::cout << "  [C++17] 范围 for + 结构化绑定：\n";
    for (const auto& [key, value] : m) {
        std::cout << "    " << key << " -> " << value << "\n";
    }
}

void demo() {
    section("1. 容器遍历：迭代器 → range-for → 结构化绑定");

    std::map<std::string, int> scores = {
        {"Alice", 95}, {"Bob", 87}, {"Charlie", 92}
    };

    cpp98_style(scores);
    cpp11_style(scores);
    cpp17_style(scores);

    std::cout << "\n  演进：冗长的迭代器声明 → auto + range-for → 结构化绑定\n";
    std::cout << "  收益：代码更简洁，语义更清晰，减少出错可能\n";
}

} // namespace topic1_iteration

// ============================================================================
// 2. 函数返回类型推导
// ============================================================================
namespace topic2_return_type {

// 场景：一个加法函数，返回类型依赖于参数类型

// C++11：尾置返回类型 (trailing return type)
template <typename T, typename U>
auto add_cpp11(T a, U b) -> decltype(a + b) {
    return a + b;
}

// C++14：直接用 auto（编译器从 return 语句推导）
template <typename T, typename U>
auto add_cpp14(T a, U b) {
    return a + b;
}

// C++17：与 C++14 基本相同，但 constexpr 支持更好
// 另外 if constexpr 让条件编译的函数返回类型推导更自然
template <typename T>
auto process_cpp17(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else {
        return value;
    }
}

// 对比：C++11 实现相同功能需要 SFINAE 或模板特化
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process_cpp11(T value) {
    return value * 2;
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
process_cpp11(T value) {
    return value;
}

void demo() {
    section("2. 返回类型：尾置返回 → auto → if constexpr");

    std::cout << "  简单加法函数：\n";
    std::cout << "  [C++11] auto add(T a, U b) -> decltype(a+b) { return a+b; }\n";
    std::cout << "  [C++14] auto add(T a, U b) { return a+b; }\n";
    std::cout << "         省去了尾置返回类型，更简洁\n\n";

    auto r11 = add_cpp11(1, 2.5);
    auto r14 = add_cpp14(1, 2.5);
    std::cout << "  结果一致: " << r11 << " == " << r14 << "\n";

    std::cout << "\n  条件分支（类型依赖）：\n";
    std::cout << "  [C++11] 需要两个 SFINAE 函数，代码冗长且难读\n";
    std::cout << "  [C++17] if constexpr 在一个函数内完成编译期分支\n\n";

    auto int_result = process_cpp17(21);    // int -> 返回 42
    auto str_result = process_cpp17(std::string("hello"));  // string -> 返回原值
    std::cout << "  process(21) = " << int_result << " (int 类型，乘以2)\n";
    std::cout << "  process(\"hello\") = " << str_result << " (string 类型，不变)\n";

    std::cout << "\n  演进：decltype尾置返回 → auto推导 → if constexpr编译期分支\n";
    std::cout << "  收益：代码从2个SFINAE函数压缩到1个函数，可读性大幅提升\n";
}

} // namespace topic2_return_type

// ============================================================================
// 3. 多态变体：继承 vs variant
// ============================================================================
namespace topic3_polymorphism {

// 场景：表示不同类型的形状（圆形、矩形），计算面积

// ---- C++11 方式：继承 + 虚函数 ----
class ShapeCpp11 {
public:
    virtual ~ShapeCpp11() = default;
    virtual double area() const = 0;
    virtual const char* type_name() const = 0;
};

class CircleCpp11 : public ShapeCpp11 {
public:
    CircleCpp11(double r) : radius(r) {}
    double area() const override { return 3.14159 * radius * radius; }
    const char* type_name() const override { return "Circle"; }
    double radius;
};

class RectangleCpp11 : public ShapeCpp11 {
public:
    RectangleCpp11(double w, double h) : width(w), height(h) {}
    double area() const override { return width * height; }
    const char* type_name() const override { return "Rectangle"; }
    double width, height;
};

// ---- C++17 方式：std::variant + std::visit ----
struct Circle { double radius; };
struct Rectangle { double width; double height; };
struct Triangle { double base, height; };

using Shape = std::variant<Circle, Rectangle, Triangle>;

// 面积计算 visitor（C++17 方式）
struct AreaVisitor {
    double operator()(const Circle& c) const {
        return 3.14159 * c.radius * c.radius;
    }
    double operator()(const Rectangle& r) const {
        return r.width * r.height;
    }
    double operator()(const Triangle& t) const {
        return 0.5 * t.base * t.height;
    }
};

// 更简洁的方式：overloaded lambda（C++17）
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;  // 推导指引

double total_area_cpp11(const std::vector<std::unique_ptr<ShapeCpp11>>& shapes) {
    double total = 0;
    for (const auto& s : shapes) {
        total += s->area();
    }
    return total;
}

double total_area_cpp17(const std::vector<Shape>& shapes) {
    double total = 0;
    for (const auto& s : shapes) {
        total += std::visit(AreaVisitor{}, s);
    }
    return total;
}

void demo() {
    section("3. 多态：继承虚函数 → variant + visit");

    // C++11 方式
    std::vector<std::unique_ptr<ShapeCpp11>> shapes11;
    shapes11.push_back(std::make_unique<CircleCpp11>(5.0));
    shapes11.push_back(std::make_unique<RectangleCpp11>(3.0, 4.0));
    std::cout << "  [C++11] 继承 + 虚函数：\n";
    std::cout << "    总面积 = " << total_area_cpp11(shapes11) << "\n";
    std::cout << "    特点：需要基类、虚函数、堆分配、生命周期管理\n\n";

    // C++17 方式
    std::vector<Shape> shapes17 = {
        Circle{5.0},
        Rectangle{3.0, 4.0},
        Triangle{3.0, 4.0}
    };
    std::cout << "  [C++17] std::variant + std::visit：\n";
    std::cout << "    总面积 = " << total_area_cpp17(shapes17) << "\n";
    std::cout << "    特点：值语义、栈上分配、无需继承、封闭类型集合\n\n";

    // overloaded lambda 示例
    std::cout << "  使用 overloaded lambda 直接访问：\n";
    for (const auto& s : shapes17) {
        std::visit(overloaded{
            [](const Circle& c) {
                std::cout << "    圆形，半径=" << c.radius << "\n";
            },
            [](const Rectangle& r) {
                std::cout << "    矩形，" << r.width << "x" << r.height << "\n";
            },
            [](const Triangle& t) {
                std::cout << "    三角形，底=" << t.base
                          << " 高=" << t.height << "\n";
            }
        }, s);
    }

    std::cout << "\n  对比：\n";
    std::cout << "    继承方式：开放类型集（可随时加新类），运行时多态，堆分配\n";
    std::cout << "    variant方式：封闭类型集（编译期确定），值语义，栈友好\n";
    std::cout << "  适用场景不同，但 variant 为值语义多态提供了新选择\n";
}

} // namespace topic3_polymorphism

// ============================================================================
// 4. 可选值：指针 vs optional
// ============================================================================
namespace topic4_optional {

// 场景：一个可能失败的查找函数

// C++11 方式1：返回指针（nullptr 表示没找到）
int* find_cpp11_ptr(std::vector<int>& vec, int value) {
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end()) return &(*it);
    return nullptr;
}

// C++11 方式2：返回 bool + 输出参数
bool find_cpp11_outparam(const std::vector<int>& vec, int value, int& out) {
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end()) {
        out = *it;
        return true;
    }
    return false;
}

// C++17 方式：std::optional
std::optional<int> find_cpp17(const std::vector<int>& vec, int value) {
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end()) return *it;
    return std::nullopt;
}

void demo() {
    section("4. 可选值：指针/nullptr → std::optional");

    std::vector<int> data = {10, 20, 30, 40, 50};

    std::cout << "  [C++11] 返回指针：\n";
    int* p = find_cpp11_ptr(data, 30);
    if (p) std::cout << "    找到: " << *p << "\n";
    else   std::cout << "    未找到\n";
    std::cout << "    问题：所有权不明确，可能误用空指针\n\n";

    std::cout << "  [C++11] 输出参数：\n";
    int result = 0;
    bool found = find_cpp11_outparam(data, 99, result);
    std::cout << "    找到: " << std::boolalpha << found
              << ", 值: " << result << "\n";
    std::cout << "    问题：API 不自然，result 必须先定义\n\n";

    std::cout << "  [C++17] std::optional：\n";
    auto opt = find_cpp17(data, 30);
    if (opt) std::cout << "    找到: " << *opt << "\n";
    else     std::cout << "    未找到\n";

    // value_or：有值返回值，无值返回默认值
    auto opt2 = find_cpp17(data, 99);
    std::cout << "    value_or(-1): " << opt2.value_or(-1) << "\n";

    // 和 if 初始化器结合（C++17 if 初始化）
    if (auto it = find_cpp17(data, 40); it) {
        std::cout << "    if-初始化器中找到: " << *it << "\n";
    }

    std::cout << "\n  演进：指针语义 → 值语义的可选类型\n";
    std::cout << "  收益：表达力更强、更安全、所有权清晰\n";
}

} // namespace topic4_optional

// ============================================================================
// 5. 变参求和：递归展开 → 折叠表达式
// ============================================================================
namespace topic5_fold {

// 场景：对任意数量的参数求和

// C++11：递归展开 + 基础情况
template <typename T>
T sum_cpp11(T value) {
    return value;
}

template <typename T, typename... Rest>
T sum_cpp11(T first, Rest... rest) {
    return first + sum_cpp11(rest...);
}

// C++17：折叠表达式，一行搞定
template <typename... Args>
auto sum_cpp17(Args... args) {
    return (... + args);  // 一元左折叠
}

// 更复杂的例子：打印所有参数

// C++11 方式
void print_all_cpp11() {
    std::cout << "\n";
}

template <typename T, typename... Rest>
void print_all_cpp11(T first, Rest... rest) {
    std::cout << first << " ";
    print_all_cpp11(rest...);
}

// C++17 方式：逗号折叠
template <typename... Args>
void print_all_cpp17(Args... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}

void demo() {
    section("5. 变参处理：递归展开 → 折叠表达式");

    std::cout << "  求和：\n";
    std::cout << "  [C++11] 递归模板 + 基础情况（需要2个函数）\n";
    std::cout << "    sum(1,2,3,4,5) = " << sum_cpp11(1, 2, 3, 4, 5) << "\n\n";

    std::cout << "  [C++17] 折叠表达式（一个函数一行代码）\n";
    std::cout << "    sum(1,2,3,4,5) = " << sum_cpp17(1, 2, 3, 4, 5) << "\n";
    std::cout << "    return (... + args); // 一元左折叠\n\n";

    std::cout << "  打印所有参数：\n";
    std::cout << "  [C++11] ";
    print_all_cpp11(1, "hello", 3.14, 'a');
    std::cout << "  [C++17] ";
    print_all_cpp17(1, "hello", 3.14, 'a');

    std::cout << "\n  演进：递归模板展开 → 折叠表达式\n";
    std::cout << "  收益：代码量大幅减少，编译更快，可读性更高\n";
}

} // namespace topic5_fold

// ============================================================================
// 6. 元组访问：std::get → 结构化绑定
// ============================================================================
namespace topic6_tuple {

// 场景：函数返回多个值

// C++11：返回 tuple，用 std::get 访问
std::tuple<std::string, int, double> get_person_cpp11() {
    return std::make_tuple("Alice", 30, 165.5);
}

// C++17：还是返回 tuple，但用结构化绑定接收
std::tuple<std::string, int, double> get_person_cpp17() {
    return {"Bob", 25, 180.0};  // 甚至不需要 make_tuple（CTAD）
}

void demo() {
    section("6. 多返回值：std::get → 结构化绑定");

    // C++11 方式
    auto person11 = get_person_cpp11();
    std::cout << "  [C++11] std::get<N>(tuple)：\n";
    std::cout << "    姓名: " << std::get<0>(person11) << "\n";
    std::cout << "    年龄: " << std::get<1>(person11) << "\n";
    std::cout << "    身高: " << std::get<2>(person11) << "\n";
    std::cout << "    问题：索引数字不直观，容易弄错\n\n";

    // C++17 方式
    auto [name, age, height] = get_person_cpp17();
    std::cout << "  [C++17] 结构化绑定：\n";
    std::cout << "    姓名: " << name << "\n";
    std::cout << "    年龄: " << age << "\n";
    std::cout << "    身高: " << height << "\n";
    std::cout << "    优势：变量名直接体现含义，代码自文档化\n\n";

    // C++17：结构化绑定还可以用于结构体和数组
    struct Point { double x, y, z; };
    Point p{1.0, 2.0, 3.0};
    auto [px, py, pz] = p;
    std::cout << "  结构体绑定: (" << px << ", " << py << ", " << pz << ")\n";

    int arr[] = {10, 20, 30};
    auto [a, b, c] = arr;
    std::cout << "  数组绑定: " << a << ", " << b << ", " << c << "\n";

    std::cout << "\n  演进：std::get<0>/std::get<1>/... → 有意义的变量名\n";
    std::cout << "  收益：代码可读性大幅提升，减少索引错误\n";
}

} // namespace topic6_tuple

// ============================================================================
// 7. 模板元编程：SFINAE → void_t → if constexpr
// ============================================================================
namespace topic7_tmpl_meta {

// 场景：检测一个类型是否有 serialize() 成员函数

// ---- C++11：SFINAE + decltype 检测 ----
template <typename T>
struct has_serialize_cpp11 {
private:
    template <typename U>
    static auto test(int) -> decltype(std::declval<U>().serialize(), std::true_type());
    template <typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// ---- C++17：更简洁的 void_t 方式（C++17 标准库引入，但C++11也能实现） ----
// 实际上 void_t 在 C++11 也能实现，但 C++17 正式加入标准库
template <typename...>
using void_t = void;

template <typename T, typename = void>
struct has_serialize : std::false_type {};

template <typename T>
struct has_serialize<T, void_t<decltype(std::declval<T>().serialize())>>
    : std::true_type {};

// C++17 变量模板简写
template <typename T>
constexpr bool has_serialize_v = has_serialize<T>::value;

// ---- 实际应用：根据是否有 serialize 选择不同实现 ----

// C++11：SFINAE 两个函数
template <typename T>
typename std::enable_if<has_serialize_cpp11<T>::value, std::string>::type
save_cpp11(const T& obj) {
    return obj.serialize();
}

template <typename T>
typename std::enable_if<!has_serialize_cpp11<T>::value, std::string>::type
save_cpp11(const T& obj) {
    return std::string("default_") + typeid(T).name();
}

// C++17：if constexpr 一个函数搞定
template <typename T>
std::string save_cpp17(const T& obj) {
    if constexpr (has_serialize_v<T>) {
        return obj.serialize();
    } else {
        return std::string("default_") + typeid(T).name();
    }
}

// 测试类
struct Serializable {
    std::string serialize() const { return "serialized_data"; }
};

struct NonSerializable {
    int data = 42;
};

void demo() {
    section("7. 模板元编程：SFINAE → void_t → if constexpr");

    std::cout << "  类型特征检测：\n";
    std::cout << "  [C++11] SFINAE + test(0)/test(...) 技巧\n";
    std::cout << "    has_serialize<Serializable> = "
              << std::boolalpha << has_serialize_cpp11<Serializable>::value << "\n";
    std::cout << "    has_serialize<NonSerializable> = "
              << has_serialize_cpp11<NonSerializable>::value << "\n\n";

    std::cout << "  [C++17] void_t 偏特化 + 变量模板\n";
    std::cout << "    has_serialize_v<Serializable> = "
              << has_serialize_v<Serializable> << "\n";
    std::cout << "    has_serialize_v<NonSerializable> = "
              << has_serialize_v<NonSerializable> << "\n\n";

    std::cout << "  条件分发：\n";
    Serializable s;
    NonSerializable ns;

    std::cout << "  [C++11] 两个 SFINAE 函数：\n";
    std::cout << "    save(Serializable) = " << save_cpp11(s) << "\n";
    std::cout << "    save(NonSerializable) = " << save_cpp11(ns) << "\n\n";

    std::cout << "  [C++17] if constexpr 一个函数：\n";
    std::cout << "    save(Serializable) = " << save_cpp17(s) << "\n";
    std::cout << "    save(NonSerializable) = " << save_cpp17(ns) << "\n";

    std::cout << "\n  演进：\n";
    std::cout << "    C++11: 多个 SFINAE 重载函数，晦涩难懂\n";
    std::cout << "    C++17: if constexpr 内联编译期分支，直观自然\n";
    std::cout << "  收益：模板元编程从"专家黑魔法"变为"普通开发者可用"\n";
}

} // namespace topic7_tmpl_meta

// ============================================================================
// 8. 函数式编程：bind → lambda → 泛型lambda
// ============================================================================
namespace topic8_functional {

// 场景：创建一个"加5"的函数对象

int add(int a, int b) { return a + b; }

void demo() {
    section("8. 函数式：std::bind → lambda → 泛型lambda");

    std::cout << "  [C++03/11] std::bind：\n";
    auto add5_bind = std::bind(add, std::placeholders::_1, 5);
    std::cout << "    add5_bind(10) = " << add5_bind(10) << "\n";
    std::cout << "    缺点：语法晦涩，占位符不直观，类型复杂\n\n";

    std::cout << "  [C++11] lambda：\n";
    auto add5_lambda = [](int x) { return x + 5; };
    std::cout << "    add5_lambda(10) = " << add5_lambda(10) << "\n";
    std::cout << "    优点：语法直观，可读性好\n\n";

    std::cout << "  [C++14] 泛型 lambda：\n";
    auto add5_generic = [](auto x) { return x + 5; };
    std::cout << "    add5_generic(10) = " << add5_generic(10) << " (int)\n";
    std::cout << "    add5_generic(10.5) = " << add5_generic(10.5) << " (double)\n";
    std::cout << "    add5_generic(string) = " << add5_generic(std::string("hello"))
              << " (string+int 也能工作)\n";
    std::cout << "    优点：泛型代码更简洁，无需写模板函数\n\n";

    // 实际应用：STL 算法中的谓词
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int threshold = 5;

    // C++11: bind2nd（C++11 已废弃，C++17 移除）
    // C++11 推荐用 lambda
    auto count = std::count_if(v.begin(), v.end(),
        [threshold](int x) { return x > threshold; });
    std::cout << "  大于 " << threshold << " 的个数: " << count
              << " (lambda 谓词)\n";

    // C++14 泛型 lambda 在算法中的优势
    std::vector<std::string> vs = {"apple", "banana", "cherry"};
    std::sort(vs.begin(), vs.end(),
        [](const auto& a, const auto& b) { return a.size() < b.size(); });
    std::cout << "  按长度排序字符串: ";
    for (const auto& s : vs) std::cout << s << " ";
    std::cout << "\n";

    std::cout << "\n  演进：bind → lambda → 泛型lambda\n";
    std::cout << "  收益：语法越来越直观，泛型能力越来越强\n";
}

} // namespace topic8_functional

// ============================================================================
// 9. 字符串处理：string → string_view
// ============================================================================
namespace topic9_string_view {

// 场景：解析字符串，提取子串

// C++11：每次返回 std::string（创建新字符串，有开销）
std::string get_prefix_cpp11(const std::string& str, size_t n) {
    return str.substr(0, n);  // 创建新的 string，有内存分配
}

// C++17：返回 std::string_view（零拷贝）
std::string_view get_prefix_cpp17(std::string_view str, size_t n) {
    return str.substr(0, n);  // 零拷贝，只调整指针和长度
}

// 更实际的例子：解析 CSV 行
std::vector<std::string> split_csv_cpp11(const std::string& line) {
    std::vector<std::string> result;
    size_t start = 0;
    for (size_t i = 0; i <= line.size(); i++) {
        if (i == line.size() || line[i] == ',') {
            result.push_back(line.substr(start, i - start));
            start = i + 1;
        }
    }
    return result;
}

std::vector<std::string_view> split_csv_cpp17(std::string_view line) {
    std::vector<std::string_view> result;
    size_t start = 0;
    for (size_t i = 0; i <= line.size(); i++) {
        if (i == line.size() || line[i] == ',') {
            result.push_back(line.substr(start, i - start));
            start = i + 1;
        }
    }
    return result;
}

void demo() {
    section("9. 字符串：std::string → std::string_view（零拷贝）");

    std::string data = "Hello, World! This is a test string.";

    std::cout << "  提取前缀：\n";
    std::cout << "  [C++11] 返回 string（拷贝）：\n";
    auto s11 = get_prefix_cpp11(data, 5);
    std::cout << "    \"" << s11 << "\" （新字符串，有内存分配）\n\n";

    std::cout << "  [C++17] 返回 string_view（零拷贝）：\n";
    auto s17 = get_prefix_cpp17(data, 5);
    std::cout << "    \"" << s17 << "\" （只是指针+长度，无分配）\n\n";

    std::cout << "  CSV 解析：\n";
    std::string csv = "apple,banana,cherry,date,elderberry";

    auto result11 = split_csv_cpp11(csv);
    std::cout << "  [C++11] 生成 " << result11.size()
              << " 个 string（每个都有分配）\n";

    auto result17 = split_csv_cpp17(csv);
    std::cout << "  [C++17] 生成 " << result17.size()
              << " 个 string_view（零拷贝）\n";

    std::cout << "\n  演进：string（值语义，总是拷贝）\n";
    std::cout << "        → string_view（非拥有引用，零拷贝）\n";
    std::cout << "  收益：解析/只读访问场景性能大幅提升\n";
    std::cout << "  注意：string_view 不拥有数据，需注意生命周期\n";
}

} // namespace topic9_string_view

// ============================================================================
// 10. 初始化与构造
// ============================================================================
namespace topic10_initialization {

// 场景：创建一个 pair/tuple/自定义类

template <typename T>
class MyContainer {
public:
    MyContainer(T value) : value_(std::move(value)) {}
    const T& get() const { return value_; }
private:
    T value_;
};

void demo() {
    section("10. 构造：make_* → 类模板参数推导(CTAD)");

    // pair
    std::cout << "  [C++11] std::make_pair：\n";
    auto p11 = std::make_pair(std::string("hello"), 42);
    std::cout << "    make_pair(\"hello\", 42) -> pair<string, int>\n\n";

    std::cout << "  [C++17] 类模板参数推导 (CTAD)：\n";
    std::pair p17(std::string("hello"), 42);  // 自动推导模板参数
    std::cout << "    pair(\"hello\", 42) -> pair<string, int>\n";
    std::cout << "    无需 make_pair，直接构造即可推导\n\n";

    // tuple
    std::cout << "  tuple 也是一样：\n";
    std::tuple t17(1, 2.5, std::string("three"));
    std::cout << "    tuple(1, 2.5, \"three\") 自动推导类型\n\n";

    // 自定义类的推导指引（C++17 新特性）
    // 标准库的 pair/tuple/vector 等都内置了推导指引
    std::vector v = {1, 2, 3, 4, 5};  // vector<int>
    std::cout << "  vector{1,2,3,4,5} 自动推导为 vector<int>\n";
    std::cout << "  大小: " << v.size() << "\n";

    // 自定义类也可以加推导指引
    // MyContainer(const char*) -> MyContainer<std::string>
    // （实际实现需要在类外部定义推导指引）

    std::cout << "\n  演进：make_* 辅助函数 → 类模板参数推导(CTAD)\n";
    std::cout << "  收益：变量定义更简洁，类型自动推导\n";
}

} // namespace topic10_initialization

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << R"(
============================================================
  C++11 → C++14 → C++17 演进对比
  同一功能，不同版本的实现方式对比
============================================================
)";

    topic1_iteration::demo();
    topic2_return_type::demo();
    topic3_polymorphism::demo();
    topic4_optional::demo();
    topic5_fold::demo();
    topic6_tuple::demo();
    topic7_tmpl_meta::demo();
    topic8_functional::demo();
    topic9_string_view::demo();
    topic10_initialization::demo();

    std::cout << R"(
============================================================
  总结：C++11 到 C++17 的演进趋势
============================================================

  1. 更简洁：auto、结构化绑定、CTAD 减少了冗余的类型声明
  2. 更安全：智能指针、optional、string_view 减少了常见错误
  3. 更强大：if constexpr、折叠表达式让模板元编程更易用
  4. 更高效：移动语义、string_view、variant 值语义提升性能
  5. 更现代：lambda、泛型lambda、函数式编程风格融入标准库

  C++11：现代C++的基石（革命性变化）
  C++14：完善与补充（小步迭代）
  C++17：库特性大爆发（实用性大幅提升）
============================================================
)";

    return 0;
}
