/**
 * @file structured_binding.cpp
 * @brief C++17 结构化绑定高级用法
 *
 * 知识点覆盖：
 * 1. 绑定数组、pair、tuple、struct 的各种形式
 * 2. 引用绑定和 const 绑定的语义
 * 3. 在范围 for 循环中的高级应用
 * 4. 自定义类型支持结构化绑定（tuple-like 协议）
 * 5. 结构化绑定的底层机制与性能考量
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic structured_binding.cpp -o structured_binding
 */

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <map>
#include <utility>
#include <type_traits>
#include <cmath>
#include <cassert>

// ============================================================
// 第一部分：基础绑定形式
// ============================================================

namespace basic_binding {

struct Point {
    int x;
    int y;
    std::string name;
};

struct Rectangle {
    Point top_left;
    int width;
    int height;
};

void demo_array_binding() {
    std::cout << "===== 1.1 数组绑定 =====\n";

    int arr[] = {1, 2, 3};
    auto [a, b, c] = arr;  // 绑定原生数组
    std::cout << "原生数组: a=" << a << ", b=" << b << ", c=" << c << "\n";

    std::array<double, 3> coords = {1.5, 2.5, 3.5};
    auto [x, y, z] = coords;  // 绑定 std::array
    std::cout << "std::array: x=" << x << ", y=" << y << ", z=" << z << "\n";

    // 注意：绑定数量必须与数组大小匹配
    // auto [m, n] = arr;  // 错误：数量不匹配
}

void demo_pair_binding() {
    std::cout << "\n===== 1.2 pair 绑定 =====\n";

    std::pair<int, std::string> p(42, "hello");

    // C++17 之前的写法
    int old_first = p.first;
    std::string old_second = p.second;
    std::cout << "C++14 方式: first=" << old_first
              << ", second=" << old_second << "\n";

    // C++17 结构化绑定
    auto [first, second] = p;
    std::cout << "first = " << first << ", second = " << second << "\n";

    // 配合 map 插入，检查插入是否成功
    std::map<int, std::string> m;
    auto [it, inserted] = m.insert({1, "one"});
    std::cout << "插入结果: key=" << it->first
              << ", value=" << it->second
              << ", 是否新插入=" << std::boolalpha << inserted << "\n";
}

void demo_tuple_binding() {
    std::cout << "\n===== 1.3 tuple 绑定 =====\n";

    std::tuple<int, double, std::string> t(1, 3.14, "pi");

    // C++17 之前：用 std::get 逐个取出
    int i = std::get<0>(t);
    double d = std::get<1>(t);
    std::string s = std::get<2>(t);
    std::cout << "C++14 方式: i=" << i << ", d=" << d << ", s=" << s << "\n";

    // C++17 结构化绑定
    auto [ival, dval, sval] = t;
    std::cout << "C++17 方式: ival=" << ival << ", dval=" << dval << ", sval=" << sval << "\n";

    // 使用 tie 对比（C++11 已有，但需要先声明变量）
    int a; double b; std::string c;
    std::tie(a, b, c) = t;
    std::cout << "tie 方式: a=" << a << ", b=" << b << ", c=" << c << "\n";
}

void demo_struct_binding() {
    std::cout << "\n===== 1.4 struct/class 绑定 =====\n";

    Point pt{10, 20, "origin"};
    auto [px, py, pname] = pt;
    std::cout << "Point: x=" << px << ", y=" << py << ", name=" << pname << "\n";

    // 绑定嵌套结构体？不行！必须一层一层来
    Rectangle rect{{5, 5, "corner"}, 100, 200};
    auto [tl, w, h] = rect;
    // tl 的类型是 Point，可以再绑定
    auto [rx, ry, rname] = tl;
    std::cout << "Rectangle: top_left=(" << rx << "," << ry
              << "), width=" << w << ", height=" << h << "\n";

    // 注意：私有成员不能参与结构化绑定（除非有 friend 声明的特殊技巧）
}

} // namespace basic_binding


// ============================================================
// 第二部分：引用、const 与值类别
// ============================================================

namespace ref_and_const {

struct Data {
    int id;
    std::string name;
};

Data global_data{0, "global"};

Data& get_ref() { return global_data; }
const Data& get_cref() { return global_data; }
Data get_value() { return {42, "temp"}; }

void demo_value_vs_reference() {
    std::cout << "\n===== 2.1 值绑定 vs 引用绑定 =====\n";

    // 值绑定：拷贝数据，修改不影响原对象
    auto [v_id, v_name] = get_value();
    v_id = 999;
    std::cout << "值绑定: v_id=" << v_id
              << " (修改后原对象不受影响)\n";

    // 引用绑定：别名，修改会影响原对象
    auto& [r_id, r_name] = get_ref();
    r_id = 100;  // 修改 global_data.id
    std::cout << "引用绑定: r_id=" << r_id
              << ", global_data.id=" << global_data.id << "\n";

    // const 引用绑定
    const auto& [cr_id, cr_name] = get_cref();
    // cr_id = 200;  // 错误！const 引用不能修改
    std::cout << "const引用绑定: cr_id=" << cr_id << " (只读)\n";

    // 右值引用绑定（延长临时对象生命周期）
    auto&& [rr_id, rr_name] = get_value();
    std::cout << "右值引用绑定: rr_id=" << rr_id
              << ", rr_name=" << rr_name << "\n";

    // 恢复全局数据
    global_data.id = 0;
    global_data.name = "global";
}

// 2.2 结构化绑定与移动语义
void demo_move_binding() {
    std::cout << "\n===== 2.2 结构化绑定与移动语义 =====\n";

    std::pair<int, std::string> p(1, "a very long string to avoid SSO");

    // 移动绑定：std::move 整个对象
    auto [mid, mstr] = std::move(p);
    std::cout << "移动后: mid=" << mid << ", mstr=" << mstr << "\n";
    std::cout << "原 pair 的 string 状态: "
              << (p.second.empty() ? "已被移走(空)" : "仍有内容") << "\n";
    // 注意：p.first (int) 不会被"移走"，因为标量的移动就是拷贝
}

// 2.3 结构化绑定的底层：到底是什么？
// 结构化绑定本质上是创建一个隐藏的变量（可以是值、引用），
// 然后为每个名字引入对该隐藏变量成员的"别名"。
//
// 对于 auto [a, b, c] = expr;
// 等价于：
//   auto hidden = expr;
//   using a = hidden.<first_member>;  // 伪代码，实际是别名
//
// 重要：decltype(a) 反映的是成员的声明类型，而不是绑定方式

void demo_decltype_sb() {
    std::cout << "\n===== 2.3 结构化绑定的 decltype 细节 =====\n";

    struct S { int x; int& y; };
    int val = 42;
    S s{10, val};

    auto [a, b] = s;
    // a 绑定到 s.x (int)，b 绑定到 s.y (int&)
    // decltype(a) = int (成员的声明类型)
    // decltype(b) = int& (成员的声明类型，保留引用性)

    static_assert(std::is_same_v<decltype(a), int>);
    static_assert(std::is_same_v<decltype(b), int&>);

    std::cout << "decltype(a) = int (成员本身类型)\n";
    std::cout << "decltype(b) = int& (成员本身就是引用)\n";
}

} // namespace ref_and_const


// ============================================================
// 第三部分：范围 for 中的结构化绑定
// ============================================================

namespace range_for_sb {

struct Employee {
    int id;
    std::string name;
    double salary;
};

void demo_map_iteration() {
    std::cout << "\n===== 3.1 map 遍历的革命 =====\n";

    std::map<int, std::string> id_name_map = {
        {1, "Alice"},
        {2, "Bob"},
        {3, "Charlie"}
    };

    // C++14 写法：繁琐
    std::cout << "C++14 风格:\n";
    for (const auto& pair : id_name_map) {
        std::cout << "  id=" << pair.first << ", name=" << pair.second << "\n";
    }

    // C++17 写法：语义清晰
    std::cout << "C++17 结构化绑定:\n";
    for (const auto& [id, name] : id_name_map) {
        std::cout << "  id=" << id << ", name=" << name << "\n";
    }
}

void demo_vector_struct() {
    std::cout << "\n===== 3.2 vector<struct> 遍历 =====\n";

    std::vector<Employee> employees = {
        {101, "Alice", 50000.0},
        {102, "Bob", 60000.0},
        {103, "Charlie", 55000.0}
    };

    // 引用绑定，可以修改原数据
    std::cout << "加薪前:\n";
    for (const auto& [id, name, salary] : employees) {
        std::cout << "  " << name << " (ID:" << id << "): $" << salary << "\n";
    }

    // 非 const 引用，修改数据
    for (auto& [id, name, salary] : employees) {
        salary *= 1.1;  // 涨薪 10%
    }

    std::cout << "加薪后:\n";
    for (const auto& [id, name, salary] : employees) {
        std::cout << "  " << name << " (ID:" << id << "): $" << salary << "\n";
    }
}

void demo_with_index_trick() {
    std::cout << "\n===== 3.3 带索引的范围 for (C++17 技巧) =====\n";

    std::vector<std::string> fruits = {"apple", "banana", "cherry"};

    // 使用结构化绑定 + 索引计数（没有 Python 那样的 enumerate，但可以模拟）
    size_t index = 0;
    for (const auto& fruit : fruits) {
        std::cout << "[" << index << "] " << fruit << "\n";
        ++index;
    }

    // 或者使用结构数组 + 手动索引
    std::cout << "使用迭代器+距离:\n";
    for (auto it = fruits.begin(); it != fruits.end(); ++it) {
        auto idx = std::distance(fruits.begin(), it);
        std::cout << "[" << idx << "] " << *it << "\n";
    }
}

} // namespace range_for_sb


// ============================================================
// 第四部分：自定义类型支持结构化绑定（tuple-like 协议）
// ============================================================

namespace custom_sb {

// 要让自定义类型支持结构化绑定，需要提供 tuple-like 接口：
// 1. std::tuple_size<T>::value  -> 元素数量
// 2. std::tuple_element<I, T>::type -> 第 I 个元素的类型
// 3. get<I>(t) -> 获取第 I 个元素的值

// 例子：一个 3D 向量类
class Vector3D {
    double x_, y_, z_;
public:
    Vector3D(double x = 0, double y = 0, double z = 0)
        : x_(x), y_(y), z_(z) {}

    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }

    double& rx() { return x_; }
    double& ry() { return y_; }
    double& rz() { return z_; }

    double length() const { return std::sqrt(x_*x_ + y_*y_ + z_*z_); }
};

} // namespace custom_sb

// === 必须在 std 命名空间中特化 tuple_size 和 tuple_element ===
namespace std {

template <>
struct tuple_size<custom_sb::Vector3D> : integral_constant<size_t, 3> {};

template <size_t I>
struct tuple_element<I, custom_sb::Vector3D> {
    using type = double;  // 三个分量都是 double
};

// 对于 const 版本也需要特化（结构化绑定时会用到）
template <size_t I>
struct tuple_element<I, const custom_sb::Vector3D> {
    using type = double;
};

} // namespace std

namespace custom_sb {

// get 函数（放在自定义类型的命名空间中，通过 ADL 查找）
template <size_t I>
decltype(auto) get(Vector3D& v) {
    static_assert(I < 3, "Vector3D only has 3 elements");
    if constexpr (I == 0) return v.rx();
    else if constexpr (I == 1) return v.ry();
    else return v.rz();
}

template <size_t I>
decltype(auto) get(const Vector3D& v) {
    static_assert(I < 3, "Vector3D only has 3 elements");
    if constexpr (I == 0) return v.x();
    else if constexpr (I == 1) return v.y();
    else return v.z();
}

template <size_t I>
decltype(auto) get(Vector3D&& v) {
    static_assert(I < 3, "Vector3D only has 3 elements");
    if constexpr (I == 0) return v.x();
    else if constexpr (I == 1) return v.y();
    else return v.z();
}

void demo_custom_structured_binding() {
    std::cout << "\n===== 4. 自定义类型结构化绑定 =====\n";

    Vector3D v(1.0, 2.0, 3.0);

    // 值绑定
    auto [vx, vy, vz] = v;
    std::cout << "值绑定: (" << vx << ", " << vy << ", " << vz << ")\n";

    // 引用绑定（可以修改原对象）
    auto& [rx, ry, rz] = v;
    rx = 10.0;
    ry = 20.0;
    std::cout << "引用绑定修改后: v = (" << v.x() << ", " << v.y() << ", " << v.z() << ")\n";

    // const 引用绑定
    const Vector3D cv(4.0, 5.0, 6.0);
    const auto& [cx, cy, cz] = cv;
    std::cout << "const引用绑定: (" << cx << ", " << cy << ", " << cz << ")\n";

    // 验证长度
    std::cout << "向量长度: " << v.length() << "\n";
}

// 进阶：让自定义类型既能通过成员访问，又能通过结构化绑定访问
// 这在设计 API 时非常有用

} // namespace custom_sb


// ============================================================
// 第五部分：高级技巧与陷阱
// ============================================================

namespace advanced_tips {

// 5.1 忽略某些绑定项（使用下划线 _ 或其他名字）
void demo_ignore_bindings() {
    std::cout << "\n===== 5.1 忽略绑定项 =====\n";

    std::tuple<int, double, std::string, char> t(1, 3.14, "hello", 'x');

    // 只关心部分值，其他只能起个名字占位（C++ 没有专门的 ignore 语法）
    // 注意：不要用 __ 开头或含双下划线的名字，这些标识符保留给实现。
    auto [i, unused1, s, unused2] = t;
    (void)unused1;
    (void)unused2;
    std::cout << "i = " << i << ", s = " << s << "\n";
    // 注意：占位名也是普通变量，只是不参与后续计算

    // 更好的做法：使用 std::tie + std::ignore（C++11 方式）
    int ii;
    std::string ss;
    std::tie(ii, std::ignore, ss, std::ignore) = t;
    std::cout << "tie+ignore: ii = " << ii << ", ss = " << ss << "\n";
}

// 5.2 结构化绑定 + 条件语句 (C++17 if/switch 初始化语句)
void demo_if_init_sb() {
    std::cout << "\n===== 5.2 if 初始化 + 结构化绑定 =====\n";

    std::map<int, std::string> m = {{1, "one"}, {2, "two"}};

    // C++17: if 初始化语句 + 结构化绑定
    if (auto [it, success] = m.insert({3, "three"}); success) {
        std::cout << "插入成功: " << it->second << "\n";
    } else {
        std::cout << "已存在: " << it->second << "\n";
    }

    // 再次插入相同 key
    if (auto [it, success] = m.insert({1, "ONE"}); success) {
        std::cout << "插入成功: " << it->second << "\n";
    } else {
        std::cout << "已存在: " << it->second << " (插入失败)\n";
    }
}

// 5.3 陷阱：生命周期问题
void demo_lifetime_trap() {
    std::cout << "\n===== 5.3 生命周期陷阱 =====\n";

    // 陷阱：临时对象的生命周期延长只适用于直接绑定的引用
    // 结构化绑定会正确延长整个临时对象的生命周期

    struct S { int a; int b; };
    auto make_S = []() -> S { return {10, 20}; };

    // OK：整个临时对象的生命周期被延长
    const auto& [a, b] = make_S();
    std::cout << "延长生命周期: a=" << a << ", b=" << b << "\n";

    // 但是要注意：不要返回结构化绑定的引用（类似不要返回局部变量引用）
    // auto& bad = []() -> auto& { auto [x,y] = S{1,2}; return x; }();  // UB
}

// 5.4 性能考量：值绑定 vs 引用绑定
void demo_performance_notes() {
    std::cout << "\n===== 5.4 性能考量 =====\n";

    // 对于小型简单类型（如 pair<int, int>），值绑定通常没有性能问题
    // 对于大型对象，务必使用引用绑定

    struct Large {
        int id;
        std::string big_data;

        Large() : id(0), big_data(10000, 'x') {}
    };

    std::vector<Large> vec(10);

    // 好的做法：const 引用绑定，避免拷贝
    std::cout << "使用 const auto& 避免大对象拷贝\n";
    for (const auto& [id, data] : vec) {
        // 只读访问
        if (id == 0 && !data.empty()) {
            // 正确
        }
    }

    // 不好的做法：值绑定导致大对象拷贝
    // for (auto [id, data] : vec) { ... }  // 每次循环都拷贝 string

    std::cout << "遍历大对象请使用 const auto& [a, b] 形式\n";
}

} // namespace advanced_tips


// ============================================================
// 综合示例：解析配置文件的模拟
// ============================================================

namespace real_world_example {

struct ConfigEntry {
    std::string key;
    std::string value;
    std::string section;
    int line_number;
};

// 模拟解析配置文件
std::vector<ConfigEntry> parse_config(const std::string& /*content*/) {
    return {
        {"host", "127.0.0.1", "network", 1},
        {"port", "8080", "network", 2},
        {"timeout", "30", "network", 3},
        {"max_connections", "100", "network", 4},
        {"log_level", "info", "logging", 6},
        {"log_file", "/var/log/app.log", "logging", 7},
    };
}

void process_config() {
    std::cout << "\n===== 6. 综合示例：配置文件解析 =====\n";

    auto entries = parse_config("dummy_config");

    std::cout << "配置项列表:\n";
    for (const auto& [key, value, section, line] : entries) {
        std::cout << "  [" << section << "] " << key << " = " << value
                  << " (第" << line << "行)\n";
    }

    // 按 section 分组统计
    std::map<std::string, int> section_counts;
    for (const auto& [key, value, section, line] : entries) {
        (void)key; (void)value; (void)line;  // 未使用，仅为了可读性
        section_counts[section]++;
    }

    std::cout << "\n各 section 配置项数量:\n";
    for (const auto& [section, count] : section_counts) {
        std::cout << "  " << section << ": " << count << " 项\n";
    }
}

} // namespace real_world_example


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 结构化绑定高级用法\n";
    std::cout << "============================================\n";

    basic_binding::demo_array_binding();
    basic_binding::demo_pair_binding();
    basic_binding::demo_tuple_binding();
    basic_binding::demo_struct_binding();

    ref_and_const::demo_value_vs_reference();
    ref_and_const::demo_move_binding();
    ref_and_const::demo_decltype_sb();

    range_for_sb::demo_map_iteration();
    range_for_sb::demo_vector_struct();
    range_for_sb::demo_with_index_trick();

    custom_sb::demo_custom_structured_binding();

    advanced_tips::demo_ignore_bindings();
    advanced_tips::demo_if_init_sb();
    advanced_tips::demo_lifetime_trap();
    advanced_tips::demo_performance_notes();

    real_world_example::process_config();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
