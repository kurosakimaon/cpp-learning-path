/**
 * @file optional_variant_any.cpp
 * @brief C++17 std::optional / std::variant / std::any 深入用法
 *
 * 知识点覆盖：
 * 1. std::optional 高级用法：安全的"可能不存在的值"、与指针的区别、值语义
 *    - monadic 操作的 C++17 模拟（and_then / transform / or_else 等 C++23 特性）
 * 2. std::variant 深入：类型安全的联合、visitor 模式、std::visit 高级用法
 *    - 重载集技巧（overloaded lambda）、变体的替代方案对比
 * 3. std::any 类型安全转换：any_cast、bad_any_cast、与 void* 的对比
 * 4. 三者的选择策略与实际工程应用场景
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic optional_variant_any.cpp -o optional_variant_any
 */

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <any>
#include <functional>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <cmath>

// ============================================================
// 第一部分：std::optional 高级用法
// ============================================================

namespace optional_demo {

// 1.1 为什么需要 optional？
// 传统表示"无值"的方法：
// - 返回指针 + nullptr 检查（容易产生悬挂指针、所有权不明确）
// - 返回 bool + 输出参数（代码冗余，不优雅）
// - 特殊哨兵值（如 -1、空字符串，容易误用）
//
// std::optional 提供了值语义的"可选值"，安全、清晰、无所有权问题

// 传统方式：用指针表示可能失败
int* find_value_old(int* arr, size_t size, int target) {
    for (size_t i = 0; i < size; ++i) {
        if (arr[i] == target) return &arr[i];
    }
    return nullptr;  // 调用者必须检查 nullptr，且可能误用
}

// C++17 方式：用 optional
std::optional<int> find_index(const std::vector<int>& vec, int target) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) return static_cast<int>(i);
    }
    return std::nullopt;  // 语义清晰：没有找到
}

void demo_basic_optional() {
    std::cout << "===== 1.1 std::optional 基础 =====\n";

    std::vector<int> data = {10, 20, 30, 40, 50};

    auto idx1 = find_index(data, 30);
    if (idx1) {
        std::cout << "找到 30，索引 = " << *idx1 << "\n";
    } else {
        std::cout << "未找到 30\n";
    }

    auto idx2 = find_index(data, 100);
    if (idx2.has_value()) {
        std::cout << "找到 100，索引 = " << idx2.value() << "\n";
    } else {
        std::cout << "未找到 100\n";
    }

    // value_or：提供默认值
    int safe_idx = find_index(data, 100).value_or(-1);
    std::cout << "安全索引（默认 -1）: " << safe_idx << "\n";
}

// 1.2 optional 的值语义与对象生命周期
// optional 在内部存储对象（不是指针），有完整的值语义

struct HeavyObject {
    std::string name;
    std::vector<int> data;

    HeavyObject(std::string n, std::vector<int> d)
        : name(std::move(n)), data(std::move(d)) {
        std::cout << "  HeavyObject 构造: " << name << "\n";
    }

    ~HeavyObject() {
        std::cout << "  HeavyObject 析构: " << name << "\n";
    }

    // 移动构造
    HeavyObject(HeavyObject&& other) noexcept
        : name(std::move(other.name)), data(std::move(other.data)) {
        std::cout << "  HeavyObject 移动构造: " << name << "\n";
    }
};

void demo_value_semantics() {
    std::cout << "\n===== 1.2 optional 值语义与生命周期 =====\n";

    std::cout << "--- 创建有值的 optional ---\n";
    std::optional<HeavyObject> opt1 = std::in_place, "obj1", std::vector<int>{1, 2, 3};
    // std::in_place 表示原地构造，避免额外的拷贝/移动

    std::cout << "--- 重置为空 ---\n";
    opt1.reset();  // 对象被析构

    std::cout << "--- 重新赋值 ---\n";
    opt1.emplace("obj2", std::vector<int>{4, 5, 6});

    std::cout << "--- 函数结束 ---\n";
}

// 1.3 monadic 操作的 C++17 模拟
// C++23 引入了 optional 的 monadic 操作：and_then, transform, or_else
// 在 C++17 中我们可以手动模拟这些操作，展示其设计思想

// transform：如果有值，应用函数并返回新的 optional；否则返回 nullopt
template <typename T, typename Func>
auto transform(const std::optional<T>& opt, Func f)
    -> std::optional<decltype(f(*opt))> {
    if (opt) {
        return f(*opt);
    }
    return std::nullopt;
}

// and_then：如果有值，应用返回 optional 的函数（链式调用）
template <typename T, typename Func>
auto and_then(const std::optional<T>& opt, Func f)
    -> decltype(f(*opt)) {
    if (opt) {
        return f(*opt);
    }
    return std::nullopt;
}

// or_else：如果为空，执行备选操作
template <typename T, typename Func>
std::optional<T> or_else(const std::optional<T>& opt, Func f) {
    if (opt) {
        return opt;
    }
    return f();
}

// 实际应用场景：安全的配置读取
struct Config {
    std::optional<int> port;
    std::optional<std::string> host;
    std::optional<int> timeout;
};

std::optional<Config> load_config(const std::string& filename) {
    // 模拟：根据文件名加载配置
    if (filename.empty()) return std::nullopt;

    Config cfg;
    cfg.port = 8080;
    cfg.host = "localhost";
    // timeout 不设置（保持 nullopt）
    return cfg;
}

int get_port_with_default(const std::string& filename) {
    auto cfg = load_config(filename);
    // 链式调用（C++23 风格）：cfg.port.value_or(80)
    // C++17 我们手动实现

    if (cfg && cfg->port) {
        return *cfg->port;
    }
    return 80;  // 默认端口
}

void demo_monadic_simulation() {
    std::cout << "\n===== 1.3 Monadic 操作模拟（C++23 前瞻） =====\n";

    // 简单的 transform 例子
    std::optional<int> num = 21;
    auto doubled = transform(num, [](int x) { return x * 2; });
    std::cout << "transform(21, x2) = " << doubled.value_or(0) << "\n";

    std::optional<int> nothing;
    auto doubled_nothing = transform(nothing, [](int x) { return x * 2; });
    std::cout << "transform(nullopt, x2) = "
              << (doubled_nothing ? "有值" : "无值") << "\n";

    // and_then 链式调用
    auto safe_sqrt = [](double x) -> std::optional<double> {
        if (x >= 0) return std::sqrt(x);
        return std::nullopt;
    };

    std::optional<double> val = 16.0;
    auto result = and_then(val, safe_sqrt);
    std::cout << "and_then(16.0, sqrt) = " << result.value_or(-1) << "\n";

    val = -4.0;
    result = and_then(val, safe_sqrt);
    std::cout << "and_then(-4.0, sqrt) = "
              << (result ? "有值" : "无值(负数无平方根)") << "\n";

    // 配置读取的实际应用
    std::cout << "配置文件端口: " << get_port_with_default("app.cfg") << "\n";
    std::cout << "空文件名端口(默认): " << get_port_with_default("") << "\n";
}

// 1.4 optional 的正确使用场景与反模式
void demo_good_practices() {
    std::cout << "\n===== 1.4 最佳实践与反模式 =====\n";

    // 好的做法：表示"可能不存在的值"
    // 如：查找结果、解析结果、可选参数

    // 不好的做法1：用 optional<bool> 代替三态（用 enum 更清晰）
    std::optional<bool> tribool = false;
    // 三种状态: true, false, nullopt
    // 建议：定义 enum class TriState { Yes, No, Unknown };

    // 不好的做法2：optional 存动态分配的指针（冗余）
    // std::optional<std::unique_ptr<T>> — 多余，unique_ptr 本身就可以是 null

    // 好的做法3：optional 与引用
    // C++ 标准中没有 optional 引用（有争议）
    // 可以用 std::reference_wrapper 或指针代替
    int x = 42;
    std::optional<std::reference_wrapper<int>> opt_ref = std::ref(x);
    if (opt_ref) {
        opt_ref->get() = 100;  // 修改原始值
    }
    std::cout << "optional 引用: x = " << x
              << " (通过 reference_wrapper 实现)\n";

    std::cout << "建议：需要可选引用时，用指针或 reference_wrapper\n";
}

} // namespace optional_demo


// ============================================================
// 第二部分：std::variant 深入
// ============================================================

namespace variant_demo {

// 2.1 variant 是什么？
// std::variant 是类型安全的联合（tagged union）
// 与 union 不同，variant 知道当前存储的是哪种类型

// 传统 union 的问题：
// - 不知道当前哪个成员是活跃的
// - 不能存储非平凡类型（如 string）
// - 容易误用（访问错误的成员）

struct Circle {
    double radius;
    Circle(double r) : radius(r) {}
    double area() const { return 3.14159 * radius * radius; }
};

struct Rectangle {
    double width;
    double height;
    Rectangle(double w, double h) : width(w), height(h) {}
    double area() const { return width * height; }
};

struct Triangle {
    double base;
    double height;
    Triangle(double b, double h) : base(b), height(h) {}
    double area() const { return 0.5 * base * height; }
};

using Shape = std::variant<Circle, Rectangle, Triangle>;

void demo_basic_variant() {
    std::cout << "\n===== 2.1 std::variant 基础 =====\n";

    // 存储不同类型
    Shape shape1 = Circle(5.0);
    Shape shape2 = Rectangle(4.0, 6.0);
    Shape shape3 = Triangle(3.0, 4.0);

    // 检查当前类型
    std::cout << "shape1 是 Circle? " << std::boolalpha
              << std::holds_alternative<Circle>(shape1) << "\n";

    // 获取值（类型不匹配会抛出 bad_variant_access）
    try {
        auto& rect = std::get<Rectangle>(shape2);
        std::cout << "Rectangle 面积: " << rect.area() << "\n";
    } catch (const std::bad_variant_access& e) {
        std::cout << "类型错误: " << e.what() << "\n";
    }

    // 用指针方式获取（不抛异常，失败返回 nullptr）
    if (auto* tri = std::get_if<Triangle>(&shape3)) {
        std::cout << "Triangle 面积: " << tri->area() << "\n";
    }

    // index() 获取当前类型的索引
    std::cout << "shape1 的类型索引: " << shape1.index()
              << " (0=Circle, 1=Rectangle, 2=Triangle)\n";
}

// 2.2 std::visit 与访问者模式
// std::visit 是 variant 最强大的功能

// 方式1：定义显式的访问者类
struct AreaVisitor {
    double operator()(const Circle& c) const { return c.area(); }
    double operator()(const Rectangle& r) const { return r.area(); }
    double operator()(const Triangle& t) const { return t.area(); }
};

// 方式2：使用 overloaded 技巧（lambda 访问者）
// 这是 C++17 variant 的经典惯用法
template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

// 推导指引（C++17 CTAD）
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

double calculate_area(const Shape& shape) {
    return std::visit(AreaVisitor{}, shape);
}

void demo_visitor_pattern() {
    std::cout << "\n===== 2.2 std::visit 访问者模式 =====\n";

    std::vector<Shape> shapes = {
        Circle(5.0),
        Rectangle(4.0, 6.0),
        Triangle(3.0, 4.0),
        Circle(2.5),
        Rectangle(3.0, 7.0)
    };

    // 使用显式访问者
    std::cout << "使用 AreaVisitor 计算面积:\n";
    for (const auto& shape : shapes) {
        std::cout << "  面积 = " << calculate_area(shape) << "\n";
    }

    // 使用 overloaded lambda（更灵活，就地定义）
    std::cout << "\n使用 Overloaded lambda 打印类型和面积:\n";
    for (const auto& shape : shapes) {
        std::visit(Overloaded{
            [](const Circle& c) {
                std::cout << "  Circle(r=" << c.radius << ") 面积=" << c.area() << "\n";
            },
            [](const Rectangle& r) {
                std::cout << "  Rectangle(" << r.width << "x" << r.height
                          << ") 面积=" << r.area() << "\n";
            },
            [](const Triangle& t) {
                std::cout << "  Triangle(base=" << t.base << ",h=" << t.height
                          << ") 面积=" << t.area() << "\n";
            }
        }, shape);
    }
}

// 2.3 std::visit 的高级用法：多 variant 访问
// std::visit 可以同时访问多个 variant，根据组合分派

using Number = std::variant<int, double>;

struct AddVisitor {
    Number operator()(int a, int b) const { return a + b; }
    Number operator()(int a, double b) const { return a + b; }
    Number operator()(double a, int b) const { return a + b; }
    Number operator()(double a, double b) const { return a + b; }
};

Number add_numbers(const Number& a, const Number& b) {
    return std::visit(AddVisitor{}, a, b);
}

void demo_multi_variant_visit() {
    std::cout << "\n===== 2.3 多 variant 访问 =====\n";

    Number n1 = 10;       // int
    Number n2 = 3.14;     // double

    auto result = add_numbers(n1, n2);
    std::visit(Overloaded{
        [](int v) { std::cout << "结果: int = " << v << "\n"; },
        [](double v) { std::cout << "结果: double = " << v << "\n"; }
    }, result);
}

// 2.4 variant 与继承的对比
// 什么时候用 variant，什么时候用虚函数继承？
//
// variant 适合：
// - 类型数量固定且已知（封闭集合）
// - 操作可以经常新增
// - 值语义，不需要指针间接
// - 性能敏感（避免虚函数开销和堆分配）
//
// 继承适合：
// - 类型可以无限扩展（开放集合）
// - 操作数量相对固定
// - 需要运行时多态和接口抽象

// 2.5 variant 的替代方案：std::monostate
// 当 variant 需要表示"空"状态时，使用 std::monostate
using MaybeInt = std::variant<std::monostate, int>;

void print_maybe_int(const MaybeInt& m) {
    std::visit(Overloaded{
        [](std::monostate) { std::cout << "无值"; },
        [](int v) { std::cout << "值 = " << v; }
    }, m);
    std::cout << "\n";
}

void demo_monostate() {
    std::cout << "\n===== 2.4 std::monostate 空状态 =====\n";

    MaybeInt m1;  // 默认构造为第一个备选类型 monostate
    MaybeInt m2 = 42;

    std::cout << "m1: ";
    print_maybe_int(m1);
    std::cout << "m2: ";
    print_maybe_int(m2);

    std::cout << "variant 大小(monostate + int): " << sizeof(MaybeInt)
              << " 字节\n";
}

} // namespace variant_demo


// ============================================================
// 第三部分：std::any 深入
// ============================================================

namespace any_demo {

// 3.1 std::any 是什么？
// std::any 可以存储任意可复制构造的类型
// 与 void* 不同，any 是类型安全的（访问时必须知道正确的类型）

void demo_basic_any() {
    std::cout << "\n===== 3.1 std::any 基础 =====\n";

    std::any a;  // 空的 any
    std::cout << "空 any has_value? " << std::boolalpha << a.has_value() << "\n";

    a = 42;  // 存储 int
    std::cout << "存储 int 后 has_value? " << a.has_value() << "\n";
    std::cout << "type = " << a.type().name() << "\n";

    a = std::string("hello");  // 存储 string
    std::cout << "存储 string 后 type = " << a.type().name() << "\n";

    // any_cast 获取值（类型不匹配会抛出 bad_any_cast）
    try {
        std::string s = std::any_cast<std::string>(a);
        std::cout << "值 = " << s << "\n";
    } catch (const std::bad_any_cast& e) {
        std::cout << "类型转换失败: " << e.what() << "\n";
    }

    // 指针版本的 any_cast（不抛异常）
    a = 3.14;
    if (double* p = std::any_cast<double>(&a)) {
        std::cout << "double 值 = " << *p << "\n";
    }
    if (int* p = std::any_cast<int>(&a)) {
        std::cout << "int 值 = " << *p << "\n";
    } else {
        std::cout << "不是 int 类型\n";
    }

    a.reset();
    std::cout << "reset 后 has_value? " << a.has_value() << "\n";
}

// 3.2 any 的实现原理与性能考量
// any 通常使用小对象优化（SSO）：小对象直接存在内部，大对象堆分配

struct SmallObj {
    int x, y;
    SmallObj(int a, int b) : x(a), y(b) {}
};

struct LargeObj {
    std::string name;
    std::vector<int> data;

    LargeObj(std::string n, std::vector<int> d)
        : name(std::move(n)), data(std::move(d)) {}
};

void demo_performance_notes() {
    std::cout << "\n===== 3.2 any 性能考量 =====\n";

    std::cout << "sizeof(std::any) = " << sizeof(std::any) << " 字节\n";
    std::cout << "sizeof(SmallObj) = " << sizeof(SmallObj) << " 字节\n";
    std::cout << "sizeof(LargeObj) = " << sizeof(LargeObj) << " 字节\n";

    std::cout << "小对象（小于 any 内部缓冲）通常直接存储，无堆分配\n";
    std::cout << "大对象需要堆分配，有性能开销\n";
}

// 3.3 any 的实际应用场景
// - 消息传递系统（消息体可以是任意类型）
// - 配置系统（配置值可以是不同类型）
// - 插件系统的类型擦除

// 场景：通用配置存储
class ConfigStore {
    std::map<std::string, std::any> data_;
public:
    template <typename T>
    void set(const std::string& key, T&& value) {
        data_[key] = std::forward<T>(value);
    }

    template <typename T>
    std::optional<T> get(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) return std::nullopt;

        if (auto* p = std::any_cast<T>(&it->second)) {
            return *p;
        }
        return std::nullopt;  // 类型不匹配
    }

    bool contains(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    const std::type_info& type_of(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) return typeid(void);
        return it->second.type();
    }
};

void demo_config_store() {
    std::cout << "\n===== 3.3 实际应用：通用配置存储 =====\n";

    ConfigStore config;
    config.set("port", 8080);
    config.set("host", std::string("localhost"));
    config.set("timeout", 30.5);
    config.set("debug_mode", true);

    // 读取配置
    auto port = config.get<int>("port");
    std::cout << "port = " << port.value_or(-1) << "\n";

    auto host = config.get<std::string>("host");
    std::cout << "host = " << host.value_or("unknown") << "\n";

    // 类型不匹配时返回 nullopt
    auto wrong_type = config.get<std::string>("port");
    std::cout << "port 作为 string 读取: "
              << (wrong_type ? "成功" : "失败(类型不匹配)") << "\n";

    // 不存在的键
    auto missing = config.get<int>("missing_key");
    std::cout << "missing_key: "
              << (missing ? "存在" : "不存在") << "\n";

    // 查询类型
    std::cout << "timeout 的类型: " << config.type_of("timeout").name() << "\n";
}

// 3.4 any vs variant vs optional 的选择
void demo_comparison() {
    std::cout << "\n===== 3.4 三者对比与选择策略 =====\n";
    std::cout << "std::optional:\n";
    std::cout << "  - 一个类型，可能有值或无值\n";
    std::cout << "  - 零开销抽象（与直接存值几乎一样）\n";
    std::cout << "  - 适用：可能不存在的返回值、可选参数\n\n";

    std::cout << "std::variant:\n";
    std::cout << "  - 固定数量的已知类型中的一种\n";
    std::cout << "  - 编译期确定类型集合，类型安全\n";
    std::cout << "  - 适用：状态机、表达式树、替代继承\n\n";

    std::cout << "std::any:\n";
    std::cout << "  - 任意类型（运行时才能确定）\n";
    std::cout << "  - 有类型擦除开销，需要 any_cast 访问\n";
    std::cout << "  - 适用：通用存储、消息系统、动态类型接口\n";
}

} // namespace any_demo


// ============================================================
// 第四部分：综合示例 - 表达式计算器
// ============================================================

namespace expression_evaluator {

// 使用 variant 实现表达式树（类型安全的 AST）

struct Number { double value; };
struct Variable { std::string name; };

struct Add;
struct Subtract;
struct Multiply;
struct Divide;

using Expr = std::variant<
    Number,
    Variable,
    std::shared_ptr<Add>,
    std::shared_ptr<Subtract>,
    std::shared_ptr<Multiply>,
    std::shared_ptr<Divide>
>;

struct Add { Expr left; Expr right; };
struct Subtract { Expr left; Expr right; };
struct Multiply { Expr left; Expr right; };
struct Divide { Expr left; Expr right; };

// 辅助构造函数
Expr make_number(double v) { return Number{v}; }
Expr make_var(std::string n) { return Variable{std::move(n)}; }
Expr make_add(Expr l, Expr r) { return std::make_shared<Add>(Add{std::move(l), std::move(r)}); }
Expr make_sub(Expr l, Expr r) { return std::make_shared<Subtract>(Subtract{std::move(l), std::move(r)}); }
Expr make_mul(Expr l, Expr r) { return std::make_shared<Multiply>(Multiply{std::move(l), std::move(r)}); }
Expr make_div(Expr l, Expr r) { return std::make_shared<Divide>(Divide{std::move(l), std::move(r)}); }

// 使用 visitor 计算表达式值
class Evaluator {
    const std::map<std::string, double>& variables_;
public:
    explicit Evaluator(const std::map<std::string, double>& vars) : variables_(vars) {}

    double operator()(const Number& num) const {
        return num.value;
    }

    double operator()(const Variable& var) const {
        auto it = variables_.find(var.name);
        if (it == variables_.end()) {
            throw std::runtime_error("未知变量: " + var.name);
        }
        return it->second;
    }

    double operator()(const std::shared_ptr<Add>& add) const {
        return std::visit(*this, add->left) + std::visit(*this, add->right);
    }

    double operator()(const std::shared_ptr<Subtract>& sub) const {
        return std::visit(*this, sub->left) - std::visit(*this, sub->right);
    }

    double operator()(const std::shared_ptr<Multiply>& mul) const {
        return std::visit(*this, mul->left) * std::visit(*this, mul->right);
    }

    double operator()(const std::shared_ptr<Divide>& div) const {
        double right = std::visit(*this, div->right);
        if (right == 0) throw std::runtime_error("除零错误");
        return std::visit(*this, div->left) / right;
    }
};

// 使用 visitor 打印表达式
class Printer {
public:
    std::string operator()(const Number& num) const {
        return std::to_string(num.value);
    }

    std::string operator()(const Variable& var) const {
        return var.name;
    }

    std::string operator()(const std::shared_ptr<Add>& add) const {
        return "(" + std::visit(*this, add->left) + " + " + std::visit(*this, add->right) + ")";
    }

    std::string operator()(const std::shared_ptr<Subtract>& sub) const {
        return "(" + std::visit(*this, sub->left) + " - " + std::visit(*this, sub->right) + ")";
    }

    std::string operator()(const std::shared_ptr<Multiply>& mul) const {
        return "(" + std::visit(*this, mul->left) + " * " + std::visit(*this, mul->right) + ")";
    }

    std::string operator()(const std::shared_ptr<Divide>& div) const {
        return "(" + std::visit(*this, div->left) + " / " + std::visit(*this, div->right) + ")";
    }
};

void demo_expression_evaluator() {
    std::cout << "\n===== 4. 综合示例：表达式计算器 =====\n";

    // 构建表达式: (x + 2) * (y - 3)
    Expr expr = make_mul(
        make_add(make_var("x"), make_number(2)),
        make_sub(make_var("y"), make_number(3))
    );

    // 打印表达式
    std::cout << "表达式: " << std::visit(Printer{}, expr) << "\n";

    // 计算表达式值
    std::map<std::string, double> vars = {{"x", 10.0}, {"y", 5.0}};
    try {
        double result = std::visit(Evaluator{vars}, expr);
        std::cout << "x=10, y=5 时结果: " << result << "\n";
        std::cout << "验证: (10+2)*(5-3) = 12 * 2 = 24\n";
    } catch (const std::exception& e) {
        std::cout << "计算错误: " << e.what() << "\n";
    }

    // 除零测试
    Expr div_by_zero = make_div(make_number(10), make_number(0));
    std::cout << "表达式: " << std::visit(Printer{}, div_by_zero) << "\n";
    try {
        std::visit(Evaluator{vars}, div_by_zero);
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << " (正确捕获)\n";
    }
}

} // namespace expression_evaluator


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 optional / variant / any 深入用法\n";
    std::cout << "============================================\n";

    optional_demo::demo_basic_optional();
    optional_demo::demo_value_semantics();
    optional_demo::demo_monadic_simulation();
    optional_demo::demo_good_practices();

    variant_demo::demo_basic_variant();
    variant_demo::demo_visitor_pattern();
    variant_demo::demo_multi_variant_visit();
    variant_demo::demo_monostate();

    any_demo::demo_basic_any();
    any_demo::demo_performance_notes();
    any_demo::demo_config_store();
    any_demo::demo_comparison();

    expression_evaluator::demo_expression_evaluator();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
