/**
 * @file type_deduction.cpp
 * @brief C++17 类型推导高级特性深度解析
 *
 * 知识点覆盖：
 * 1. auto 类型推导的进阶规则（引用折叠、cv 限定符、braced-init-list）
 * 2. decltype(auto) 与 auto 的本质区别及适用场景
 * 3. 类模板参数推导 (Class Template Argument Deduction, CTAD)
 * 4. 模板 auto 参数（非类型模板参数使用 auto）
 * 5. 用户自定义推导指引 (User-defined Deduction Guide)
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic type_deduction.cpp -o type_deduction
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <type_traits>
#include <initializer_list>
#include <utility>
#include <memory>

// ============================================================
// 第一部分：auto 类型推导进阶
// ============================================================

namespace auto_deduction {

// 辅助工具：打印类型名（编译期通过错误信息查看，或使用 typeid 做运行时近似）
template <typename T>
struct TypePrinter;  // 故意不定义，用于通过编译错误查看 T 的类型

// 1.1 auto 推导的三条规则（与模板参数推导几乎一致，除了初始化列表）

int  getValue() { return 42; }
int& getRef()   { static int x = 100; return x; }
const int& getConstRef() { static int x = 200; return x; }
int&& getRvalueRef() {
    static int x = 300;
    return std::move(x);  // 返回静态对象的右值引用，避免悬垂引用
}

void demo_basic_auto() {
    std::cout << "===== 1. auto 基础推导规则 =====\n";

    // 规则1: 用值类型推导（非引用、非指针） -> 忽略引用和顶层const/volatile
    auto a1 = getValue();     // a1: int
    auto a2 = getRef();       // a2: int  (引用被忽略)
    auto a3 = getConstRef();  // a3: int  (引用和顶层const都被忽略)
    auto a4 = getRvalueRef(); // a4: int  (右值引用也被忽略)

    std::cout << "a1 = " << a1 << ", a2 = " << a2
              << ", a3 = " << a3 << ", a4 = " << a4 << "\n";

    // 规则2: 用 auto& 推导 -> 保留const，不允许将右值绑定到非const左值引用
    const auto& b1 = getValue();      // b1: const int& (const左值引用可以绑定右值，延长生命周期)
    auto& b2 = getRef();              // b2: int&
    const auto& b3 = getConstRef();   // b3: const int&
    // auto& b4 = getRvalueRef();     // 错误！非const左值引用不能绑定右值

    std::cout << "b1 = " << b1 << ", b2 = " << b2 << ", b3 = " << b3 << "\n";

    // 规则3: 用 auto&& 推导（万能引用/转发引用） -> 引用折叠
    auto&& c1 = getValue();       // c1: int&&  (传入右值 -> 右值引用)
    auto&& c2 = getRef();         // c2: int&   (传入左值 -> 左值引用，折叠规则: & + && = &)
    auto&& c3 = getConstRef();    // c3: const int&
    int x = 10;
    auto&& c4 = x;                // c4: int&   (x是左值)
    auto&& c5 = std::move(x);     // c5: int&&  (std::move将x转为右值)

    std::cout << "c1 = " << c1 << ", c2 = " << c2
              << ", c3 = " << c3 << ", c4 = " << c4 << ", c5 = " << c5 << "\n";
}

// 1.2 auto 与 braced-init-list（花括号初始化列表）
// 这是 auto 推导与模板参数推导唯一不一致的地方！
void demo_braced_init() {
    std::cout << "\n===== 1.2 auto 与 braced-init-list =====\n";

    // auto 推导花括号时，会推导出 std::initializer_list<T>
    auto il1 = {1, 2, 3};       // il1: std::initializer_list<int>
    // auto il2 = {1, 2.0};     // 错误！类型不一致

    // N3922 修正：直接用 = 单个值的情况
    auto il3 = {42};            // il3: std::initializer_list<int> (C++11/14/17 都成立)
    auto il4{42};               // il4: int (N3922 之后；C++17 纳入标准)
                                // C++11/14 的原始规则曾是 initializer_list<int>；
                                // 但很多编译器通过缺陷报告 N3922 提前采用了新规则，
                                // 所以不要依赖旧标准下的具体行为。

    std::cout << "il1 size = " << il1.size()
              << ", il3 size = " << il3.size()
              << ", il4 = " << il4 << " (il4 是 int，不是 initializer_list)\n";

    // 注意：模板参数推导不会把裸花括号列表推成 initializer_list<T>。
    // template<typename T> void f(T t); f({1,2,3}); // 错误！
    // 但形参本身是 std::initializer_list<T> 时可以推导：
    // template<typename T> void g(std::initializer_list<T>); g({1,2,3}); // OK
}

// 1.3 auto 在函数返回值中的限制（C++14 起允许 auto 返回类型）
auto add(int a, int b) {
    return a + b;  // 返回类型推导为 int
}

// 注意：多个 return 语句必须推导为同一类型
// auto f(bool flag) {
//     if (flag) return 1;
//     else return 2.0;  // 错误！推导不一致
// }

} // namespace auto_deduction


// ============================================================
// 第二部分：decltype(auto) 的高级用法
// ============================================================

namespace decltype_auto {

// 2.1 decltype(auto) 保留值类别和引用性
// auto 会丢弃引用和顶层const，decltype(auto) 则完全保留

int  global_val = 10;
int& global_ref = global_val;
const int& global_cref = global_val;

auto get_with_auto() { return global_ref; }       // 返回 int (丢弃引用)
decltype(auto) get_with_decltype_auto() { return global_ref; }  // 返回 int& (保留引用)

auto get_cref_auto() { return global_cref; }      // 返回 int (丢弃const和引用)
decltype(auto) get_cref_dauto() { return global_cref; } // 返回 const int&

void demo_decltype_auto_basic() {
    std::cout << "\n===== 2.1 decltype(auto) 基础 =====\n";

    auto r1 = get_with_auto();
    decltype(auto) r2 = get_with_decltype_auto();

    std::cout << "get_with_auto() 返回类型推导为 int, 值 = " << r1 << "\n";
    std::cout << "get_with_decltype_auto() 返回类型为 int&, 值 = " << r2 << "\n";

    // 验证：可以通过 r2 修改 global_val
    // r2 = 999;  // 如果取消注释，global_val 会变成 999

    static_assert(std::is_same_v<decltype(get_with_auto()), int>);
    static_assert(std::is_same_v<decltype(get_with_decltype_auto()), int&>);
    std::cout << "static_assert 验证通过：auto 返回值丢弃引用，decltype(auto) 保留\n";
}

// 2.2 decltype(auto) 在泛型代码中的价值：完美转发返回值
// 在编写泛型包装函数时，我们需要保持被包装函数的值类别

template <typename Func, typename... Args>
decltype(auto) forward_call(Func&& f, Args&&... args) {
    return std::forward<Func>(f)(std::forward<Args>(args)...);
}

// 对比：如果用 auto 作为返回类型
template <typename Func, typename... Args>
auto forward_call_bad(Func&& f, Args&&... args) {
    return std::forward<Func>(f)(std::forward<Args>(args)...);
}

int&  get_int_ref(int& x) { return x; }
// 返回形参的右值引用：仅用于演示返回类型推导。
// 若实参是临时对象，函数返回后该引用立即悬空，不要在实际代码中使用返回值。
int&& get_int_rref(int&& x) { return std::move(x); }

void demo_perfect_forward_return() {
    std::cout << "\n===== 2.2 decltype(auto) 完美转发返回值 =====\n";

    int x = 42;
    // forward_call 保留了引用性
    static_assert(std::is_same_v<decltype(forward_call(get_int_ref, x)), int&>);
    static_assert(std::is_same_v<decltype(forward_call(get_int_rref, 100)), int&&>);

    // forward_call_bad 会丢失引用性
    static_assert(std::is_same_v<decltype(forward_call_bad(get_int_ref, x)), int>);

    std::cout << "forward_call(get_int_ref, x) 返回 int& (完美转发)\n";
    std::cout << "forward_call_bad(get_int_ref, x) 返回 int (丢失引用)\n";
}

// 2.3 decltype 的特殊规则：表达式 vs 变量名
// decltype(entity) -> entity 的声明类型
// decltype(expression) -> 根据表达式的值类别决定：
//   - 左值表达式 -> T&
//   - 将亡值(xvalue) -> T&&
//   - 纯右值(prvalue) -> T
void demo_decltype_rules() {
    std::cout << "\n===== 2.3 decltype 推导规则详解 =====\n";

    int a = 0;
    int* p = &a;

    // 变量名（entity）-> 声明类型
    static_assert(std::is_same_v<decltype(a), int>);

    // 括号表达式：(a) 是左值表达式 -> int&
    static_assert(std::is_same_v<decltype((a)), int&>);

    // *p 是左值表达式（解引用结果是左值）-> int&
    static_assert(std::is_same_v<decltype(*p), int&>);

    // a + 0 是纯右值 -> int
    static_assert(std::is_same_v<decltype(a + 0), int>);

    // std::move(a) 是将亡值 -> int&&
    static_assert(std::is_same_v<decltype(std::move(a)), int&&>);

    std::cout << "decltype(a)       -> int     (变量名)\n";
    std::cout << "decltype((a))     -> int&    (括号左值表达式)\n";
    std::cout << "decltype(*p)      -> int&    (解引用左值)\n";
    std::cout << "decltype(a + 0)   -> int     (纯右值)\n";
    std::cout << "decltype(move(a)) -> int&&   (将亡值)\n";
}

} // namespace decltype_auto


// ============================================================
// 第三部分：类模板参数推导 (CTAD)
// ============================================================

namespace ctad_demo {

// 3.1 标准库的 CTAD 例子
void demo_std_ctad() {
    std::cout << "\n===== 3.1 标准库 CTAD 例子 =====\n";

    // C++17 之前必须写 std::pair<int, std::string>
    std::pair p(1, std::string("hello"));  // 推导出 pair<int, string>
    static_assert(std::is_same_v<decltype(p), std::pair<int, std::string>>);

    std::tuple t(1, 2.5, std::string("hi"));  // tuple<int, double, string>
    static_assert(std::is_same_v<decltype(t), std::tuple<int, double, std::string>>);

    std::vector v = {1, 2, 3, 4, 5};  // vector<int>
    static_assert(std::is_same_v<decltype(v), std::vector<int>>);

    std::array arr = {1, 2, 3};  // array<int, 3>
    static_assert(std::is_same_v<decltype(arr), std::array<int, 3>>);

    std::cout << "pair<int, string> 推导成功，first = " << p.first << "\n";
    std::cout << "tuple size = " << std::tuple_size_v<decltype(t)> << "\n";
    std::cout << "vector size = " << v.size() << "\n";
    std::cout << "array size = " << arr.size() << " (类型含大小)\n";
}

// 3.2 自定义类模板的 CTAD（默认推导）
template <typename T>
class MyWrapper {
    T value_;
public:
    // 构造函数接受 T 类型值，编译器可以从构造函数参数推导 T
    explicit MyWrapper(T v) : value_(std::move(v)) {}

    T get() const { return value_; }
};

void demo_custom_ctad() {
    std::cout << "\n===== 3.2 自定义类模板 CTAD =====\n";

    MyWrapper w1(42);        // 推导出 MyWrapper<int>
    MyWrapper w2(3.14);      // 推导出 MyWrapper<double>
    MyWrapper w3(std::string("test"));  // MyWrapper<string>

    static_assert(std::is_same_v<decltype(w1), MyWrapper<int>>);
    static_assert(std::is_same_v<decltype(w2), MyWrapper<double>>);

    std::cout << "MyWrapper<int>: " << w1.get() << "\n";
    std::cout << "MyWrapper<double>: " << w2.get() << "\n";
    std::cout << "MyWrapper<string>: " << w3.get() << "\n";
}

// 3.3 CTAD 的限制和注意事项
// - 默认构造函数无法推导（没有参数信息）
// - 初始化列表构造函数需要推导指引

template <typename T>
class MyContainer {
    std::vector<T> data_;
public:
    MyContainer() = default;  // 默认构造无法用于推导

    MyContainer(std::initializer_list<T> il) : data_(il) {}

    size_t size() const { return data_.size(); }
};

// 注意：initializer_list 构造函数的 CTAD 需要推导指引
// （C++17 标准中 initializer_list 构造函数参与 CTAD 有特殊规则，
//  实际上对于 {val1, val2, ...} 这种列表初始化形式，CTAD 是可以工作的）

void demo_ctad_limits() {
    std::cout << "\n===== 3.3 CTAD 注意事项 =====\n";

    // MyContainer c1;  // 错误！默认构造无法推导 T
    MyContainer c2 = {1, 2, 3};  // OK: 从 initializer_list 推导
    static_assert(std::is_same_v<decltype(c2), MyContainer<int>>);

    std::cout << "MyContainer 从 initializer_list 推导，size = " << c2.size() << "\n";
}

} // namespace ctad_demo


// ============================================================
// 第四部分：模板 auto 参数（非类型模板参数）
// ============================================================

namespace template_auto {

// 4.1 auto 作为非类型模板参数 (C++17 新特性)
// 之前必须写 template<typename T, T Value>，现在可以直接用 auto

template <auto Value>
struct Constant {
    static constexpr auto value = Value;
    using type = decltype(Value);
};

void demo_auto_nttp() {
    std::cout << "\n===== 4.1 auto 非类型模板参数 =====\n";

    // 自动推导非类型参数的类型
    constexpr auto c1 = Constant<42>::value;      // int
    constexpr auto c3 = Constant<'a'>::value;     // char
    // 注意：浮点数作为非类型模板参数（如 Constant<3.14>）是 C++20 才支持的

    static_assert(std::is_same_v<typename Constant<42>::type, int>);
    static_assert(std::is_same_v<typename Constant<'a'>::type, char>);

    std::cout << "Constant<42>::value = " << c1
              << ", type = int\n";
    std::cout << "Constant<'a'>::value = " << c3
              << ", type = char\n";
}

// 4.2 结合变参模板的强大用法
template <auto... Values>
struct ValueList {
    static constexpr size_t size = sizeof...(Values);
};

void demo_auto_variadic() {
    std::cout << "\n===== 4.2 auto 变参模板 =====\n";

    // 混合整数和字符的非类型参数包
    // 注意：C++17 中 double 不能作为 NTTP，ValueList<1, 2.5, 'x'> 会编译错误
    using IntCharList = ValueList<1, 2, 'a', 'b'>;

    std::cout << "ValueList<1, 2, 'a', 'b'>::size = " << IntCharList::size << "\n";
}

// 4.3 decltype(auto) 作为非类型模板参数？不行，auto 才可以
// 但可以用 auto 推导后再用 decltype 获取类型

template <auto V>
using value_type_of = decltype(V);

static_assert(std::is_same_v<value_type_of<42>, int>);

} // namespace template_auto


// ============================================================
// 第五部分：用户自定义推导指引 (Deduction Guide)
// ============================================================

namespace deduction_guide {

// 5.1 为什么需要推导指引？
// 当编译器无法正确推导，或者我们想让推导结果与构造函数不一致时使用

template <typename T>
class SmartPtr {
    T* ptr_;
public:
    // 构造函数接受裸指针
    explicit SmartPtr(T* p) : ptr_(p) {}

    ~SmartPtr() { delete ptr_; }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
};

// 自定义推导指引：从 T* 构造时推导为 SmartPtr<T>
// （实际上这个例子编译器也能自动推导，这里仅作演示语法）
template <typename T>
SmartPtr(T*) -> SmartPtr<T>;

void demo_basic_guide() {
    std::cout << "\n===== 5.1 基本推导指引 =====\n";

    SmartPtr sp(new int(42));  // 推导出 SmartPtr<int>
    std::cout << "SmartPtr<int>: *sp = " << *sp << "\n";
}

// 5.2 更有意义的例子：迭代器范围构造
template <typename T>
class MyVector {
    std::vector<T> data_;
public:
    MyVector(std::initializer_list<T> il) : data_(il) {}

    template <typename Iter>
    MyVector(Iter first, Iter last) : data_(first, last) {}

    size_t size() const { return data_.size(); }
};

// 问题：迭代器构造函数中 T 无法从 Iter 直接推导
// 需要推导指引
template <typename Iter>
MyVector(Iter, Iter) -> MyVector<typename std::iterator_traits<Iter>::value_type>;

void demo_iterator_guide() {
    std::cout << "\n===== 5.2 迭代器推导指引 =====\n";

    std::vector<int> source = {10, 20, 30, 40, 50};

    // 如果没有推导指引，这行编译失败
    MyVector mv(source.begin(), source.end());
    static_assert(std::is_same_v<decltype(mv), MyVector<int>>);

    std::cout << "MyVector 从迭代器范围推导，size = " << mv.size() << "\n";
}

// 5.3 推导指引的高级技巧：显式推导指引
// 可以用 explicit 关键字来阻止隐式转换推导

template <typename T>
class ExplicitWrapper {
    T value_;
public:
    ExplicitWrapper(T v) : value_(std::move(v)) {}

    const T& get() const { return value_; }
};

// 显式推导指引（阻止拷贝初始化时的推导）
// template <typename T>
// explicit ExplicitWrapper(T) -> ExplicitWrapper<T>;
// 注：显式推导指引在某些场景下影响拷贝初始化 = 形式的推导

// 5.4 推导指引与类型转换
// 一个实用例子：让 const char* 推导为 std::string

template <typename T>
class StringHolder {
    T value_;
public:
    StringHolder(T v) : value_(std::move(v)) {}

    const T& get() const { return value_; }
};

// 推导指引：传入 const char* 时，推导出 StringHolder<std::string>
StringHolder(const char*) -> StringHolder<std::string>;

void demo_cstring_guide() {
    std::cout << "\n===== 5.3 const char* 推导为 string =====\n";

    StringHolder sh("hello world");
    // 推导出 StringHolder<std::string> 而不是 StringHolder<const char*>
    static_assert(std::is_same_v<decltype(sh), StringHolder<std::string>>);

    std::cout << "StringHolder 类型 = std::string, 值 = " << sh.get() << "\n";
    std::cout << "size = " << sh.get().size() << " (string 的成员函数可用)\n";
}

} // namespace deduction_guide


// ============================================================
// 综合示例：类型推导在泛型编程中的协同
// ============================================================

namespace advanced_combo {

// 一个通用的工厂函数，综合运用 auto、decltype(auto)、CTAD、模板 auto 参数

template <typename T, typename... Args>
auto make_smart(Args&&... args) {
    // 这里用 auto 返回类型，返回的是 unique_ptr<T>
    // 注意：make_unique 返回的是右值，auto 推导为 unique_ptr<T>（值类型）
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// 泛型打印函数，使用 if constexpr（将在下一个文件深入讲解）
template <typename T>
void print_type_info(const T&) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整型类型\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点类型\n";
    } else if constexpr (std::is_class_v<T>) {
        std::cout << "类类型\n";
    } else {
        std::cout << "其他类型\n";
    }
}

void demo_combination() {
    std::cout << "\n===== 6. 综合应用 =====\n";

    // CTAD: pair 的自动推导
    std::pair p(10, std::string("test"));
    auto [a, b] = p;  // 结构化绑定（下一个文件详细讲）
    std::cout << "结构化绑定: a = " << a << ", b = " << b << "\n";

    // auto 与万能引用
    auto&& r = p.first;
    static_assert(std::is_same_v<decltype(r), int&>);
    std::cout << "auto&& 绑定到左值: 类型为 int&\n";

    // 类型分发
    std::cout << "int 的类型信息: ";
    print_type_info(42);
    std::cout << "string 的类型信息: ";
    print_type_info(std::string("hi"));
}

} // namespace advanced_combo


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 类型推导高级特性深度解析\n";
    std::cout << "============================================\n";

    auto_deduction::demo_basic_auto();
    auto_deduction::demo_braced_init();

    decltype_auto::demo_decltype_auto_basic();
    decltype_auto::demo_perfect_forward_return();
    decltype_auto::demo_decltype_rules();

    ctad_demo::demo_std_ctad();
    ctad_demo::demo_custom_ctad();
    ctad_demo::demo_ctad_limits();

    template_auto::demo_auto_nttp();
    template_auto::demo_auto_variadic();

    deduction_guide::demo_basic_guide();
    deduction_guide::demo_iterator_guide();
    deduction_guide::demo_cstring_guide();

    advanced_combo::demo_combination();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
