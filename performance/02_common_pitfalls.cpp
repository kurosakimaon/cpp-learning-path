// ================================================================
//  02_common_pitfalls.cpp —— 常见性能陷阱
//  那些不经意间让代码变慢的小细节
// ================================================================
//
//  徒弟啊，性能优化不只是"写快的代码"，
//  更重要的是"别写慢的代码"。
//
//  很多时候，你的代码慢，不是因为算法差，
//  而是因为一些不起眼的小细节，在偷偷吃掉性能。
//  今天师傅就带你盘点一下 C++ 里最常见的性能陷阱，
//  每一个都有正反对比，用数据说话。
//

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <memory>      // shared_ptr
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cstddef>

using namespace std;
using namespace std::chrono;

// 禁止内联：防止编译器把小函数完全优化掉，
// 让基准测试测到的确实是“调用 + 拷贝”的成本。
#if defined(__GNUC__) || defined(__clang__)
#define BENCH_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define BENCH_NOINLINE __declspec(noinline)
#else
#define BENCH_NOINLINE
#endif

// ================================================================
//  计时辅助工具
// ================================================================

inline steady_clock::time_point now() {
    return steady_clock::now();
}

inline double elapsed_ms(steady_clock::time_point s,
                         steady_clock::time_point e) {
    return duration<double, milli>(e - s).count();
}

// 防止编译器把整个循环优化掉的技巧
// volatile 写入是可见副作用，编译器不能把前面的计算全部删掉
volatile long long g_volatile_sink = 0;

template<typename T>
inline void sink(const T& x) {
    g_volatile_sink = static_cast<long long>(x);
}

// ================================================================
//  陷阱1：不必要的对象拷贝
// ================================================================
//
//  C++ 默认是值语义，传参、返回值都会拷贝对象。
//  如果对象很大（比如 string、vector、大结构体），
//  拷贝的开销不容忽视。
//
//  正确做法：
//    - 只读的参数，传 const& 而不是传值
//    - 能移动的，用 std::move 转移所有权
//    - 返回值优化（RVO/NRVO）能省就省
//

// 大对象（模拟）：一个装有 10000 个 int 的 vector，约 40 KB
struct BigObject {
    vector<int> data;
    explicit BigObject(int n = 10000) : data(n, 42) {}
};

// ❌ 反面教材：函数只读，却按值传参 —— 每次调用都要复制整个对象
BENCH_NOINLINE long long read_by_value(BigObject obj, int idx) {
    return static_cast<long long>(obj.data[static_cast<size_t>(idx)])
         + static_cast<long long>(obj.data.size());
}

// ✅ 正确做法：只读就传 const 引用，一次拷贝都没有
BENCH_NOINLINE long long read_by_ref(const BigObject& obj, int idx) {
    return static_cast<long long>(obj.data[static_cast<size_t>(idx)])
         + static_cast<long long>(obj.data.size());
}

void pitfall_copy() {
    cout << "\n========== 陷阱1：不必要的对象拷贝 ==========" << endl;

    const int N = 20000;  // 20000 次 × 40 KB ≈ 800 MB 的复制量
    BigObject obj;

    // 传值方式
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_by_value(obj, (i * 31 + 7) % 10000);
    }
    auto end = now();
    double t_value = elapsed_ms(start, end);
    sink(sum);
    cout << "  【传值方式】耗时: " << fixed << setprecision(3)
         << t_value << " ms" << endl;

    // 传引用方式
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_by_ref(obj, (i * 31 + 7) % 10000);
    }
    end = now();
    double t_ref = elapsed_ms(start, end);
    sink(sum);
    cout << "  【传引用方式】耗时: " << fixed << setprecision(3)
         << t_ref << " ms" << endl;

    if (t_ref > 0.0) {
        cout << "\n  性能差距: " << fixed << setprecision(1)
             << (t_value / t_ref) << " 倍" << endl;
    }

    cout << "\n  为什么传值慢？" << endl;
    cout << "  传值 = 每次调用都在栈上/堆上复制一份完整的 BigObject" << endl;
    cout << "  传引用 = 只传地址，函数直接读原对象，零拷贝" << endl;
    cout << endl;
    cout << "  注意：返回值通常不会额外拷贝。" << endl;
    cout << "  编译器会做 RVO/NRVO；C++17 起，返回纯右值（临时对象）" << endl;
    cout << "  更是规定必须省略拷贝（guaranteed copy elision）。" << endl;
    cout << "  所以这里的主要成本是“参数按值传递”，不是返回值。" << endl;
    cout << endl;
    cout << "  规则：只读的大对象传 const T&；需要修改时传 T&。" << endl;
    cout << "       经验法则（不是硬标准）：对象大于两三个机器字时，" << endl;
    cout << "       传引用通常更划算；int/double 这类小对象传值反而更快。" << endl;
}

// ================================================================
//  陷阱2：隐式类型转换的性能代价
// ================================================================
//
//  C++ 有很多隐式类型转换，有时候你写的代码看起来没问题，
//  但编译器在背后偷偷做了转换，可能带来性能开销。
//
//  常见的例子：
//  - int → double（运算时自动转换）
//  - const char* → std::string（传参时构造临时对象）
//  - 子类 → 父类（对象切片 + 拷贝）
//

// ❌ 反面：参数是 string，传字符串字面量会构造临时 string
BENCH_NOINLINE size_t string_length_std(const string& s) {
    return s.size();
}

// ✅ 正面：用 string_view（C++17），零拷贝
BENCH_NOINLINE size_t string_length_sv(string_view s) {
    return s.size();
}

void pitfall_implicit_conversion() {
    cout << "\n========== 陷阱2：隐式类型转换 ==========" << endl;

    const int N = 2000000;
    // 长度 64，超过常见的 SSO 阈值（通常 15 字符），
    // 这样构造临时 string 一定会发生堆分配。
    const string source(64, 'x');
    const char* raw_str = source.c_str();

    // 传 const char* 给 const string& 参数
    // → 每次调用都隐式构造一个临时 string（分配 + 拷贝 + 释放）
    auto start = now();
    size_t total = 0;
    for (int i = 0; i < N; ++i) {
        total += string_length_std(raw_str);
    }
    auto end = now();
    double t_str = elapsed_ms(start, end);
    sink(total);
    cout << "  【const string& 参数 + 传 C 字符串】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t_str << " ms" << endl;
    cout << "    （每次调用都会构造临时 string！）" << endl;

    // 用 string_view
    start = now();
    total = 0;
    for (int i = 0; i < N; ++i) {
        total += string_length_sv(raw_str);
    }
    end = now();
    double t_sv = elapsed_ms(start, end);
    sink(total);
    cout << "\n  【string_view 参数 + 传 C 字符串】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t_sv << " ms" << endl;
    cout << "    （零拷贝，只是指针 + 长度）" << endl;

    if (t_sv > 0.0) {
        cout << "\n  性能差距: " << fixed << setprecision(1)
             << (t_str / t_sv) << " 倍" << endl;
    }
    cout << endl;
    cout << "  补充说明：" << endl;
    cout << "  - 如果字符串很短（通常 ≤15 字符），std::string 会走 SSO，" << endl;
    cout << "    不分配堆内存，临时对象的代价会小很多。" << endl;
    cout << "  - string_view 不拥有数据，它只是一个“指针 + 长度”。" << endl;
    cout << "    必须保证被观察的字符串在 view 使用期间一直存活，" << endl;
    cout << "    也不能拿它去修改原字符串。" << endl;
    cout << endl;
    cout << "  启示：" << endl;
    cout << "  - 只读字符串参数，优先用 string_view（C++17）" << endl;
    cout << "  - 注意函数参数类型和你传的东西是否匹配" << endl;
    cout << "  - 隐式构造临时对象是常见的性能杀手" << endl;
}

// ================================================================
//  陷阱3：虚函数的开销
// ================================================================
//
//  虚函数很好用，多态的基石。但它不是免费的：
//  - 每个对象多一个虚表指针（8 字节）
//  - 调用时要查虚表，多一层间接
//  - 阻碍内联优化（编译器不知道调用哪个函数）
//
//  那虚函数到底有多慢？
//  答案是：单次调用几乎没差别，但如果是在热点循环里
//  频繁调用，而且原本可以内联的话，差距就大了。
//

// 普通类 + 普通函数
class NormalCalc {
public:
    int calc(int x) const { return x * 2 + 1; }
};

// 基类 + 虚函数
class BaseCalc {
public:
    virtual int calc(int x) const = 0;
    virtual ~BaseCalc() = default;
};

class DerivedCalc : public BaseCalc {
public:
    int calc(int x) const override { return x * 2 + 1; }
};

class DerivedCalc2 : public BaseCalc {
public:
    int calc(int x) const override { return x * 3 + 2; }
};

// 强制不内联的普通函数：用来单独观察“真实函数调用”的成本
BENCH_NOINLINE int calc_noinline(int x) {
    return x * 2 + 1;
}

// 通过 noinline 工厂返回对象，并在两个派生类之间做运行期选择，
// 让调用点无法确定动态类型，避免编译器去虚化（devirtualization）。
volatile int g_calc_kind = 0;

BENCH_NOINLINE BaseCalc* make_calc() {
    if (g_calc_kind == 0) return new DerivedCalc();
    return new DerivedCalc2();
}

void pitfall_virtual() {
    cout << "\n========== 陷阱3：虚函数的开销 ==========" << endl;

    const int N = 20000000;  // 两千万次调用
    vector<int> input(N);
    for (int i = 0; i < N; ++i) {
        input[i] = static_cast<int>(
            (static_cast<long long>(i) * 131 + 17) % 1000);  // 运行期数据
    }

    // 方式1：普通成员函数，可内联 → 编译器还能向量化
    NormalCalc normal;
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += normal.calc(input[i]);
    }
    auto end = now();
    double t_inline = elapsed_ms(start, end);
    sink(sum);
    cout << "  【普通成员函数（可内联）】" << endl;
    cout << "    两千万次耗时: " << fixed << setprecision(3)
         << t_inline << " ms" << endl;

    // 方式2：普通函数，强制不内联 → 隔离“函数调用”本身的成本
    sum = 0;
    start = now();
    for (int i = 0; i < N; ++i) {
        sum += calc_noinline(input[i]);
    }
    end = now();
    double t_noinline = elapsed_ms(start, end);
    sink(sum);
    cout << "\n  【普通函数（强制不内联）】" << endl;
    cout << "    两千万次耗时: " << fixed << setprecision(3)
         << t_noinline << " ms" << endl;

    // 方式3：虚函数调用（动态类型对调用点不可见，不能内联/向量化）
    BaseCalc* virt = make_calc();
    sum = 0;
    start = now();
    for (int i = 0; i < N; ++i) {
        sum += virt->calc(input[i]);
    }
    end = now();
    double t_virt = elapsed_ms(start, end);
    sink(sum);
    delete virt;
    cout << "\n  【虚函数调用】" << endl;
    cout << "    两千万次耗时: " << fixed << setprecision(3)
         << t_virt << " ms" << endl;

    if (t_noinline > 0.0) {
        cout << "\n  虚函数 / 不内联普通函数 = " << fixed << setprecision(2)
             << (t_virt / t_noinline) << " 倍" << endl;
    }
    if (t_inline > 0.0) {
        cout << "  虚函数 / 可内联普通函数 = " << fixed << setprecision(2)
             << (t_virt / t_inline) << " 倍（差距主要来自内联+向量化）"
             << endl;
    }

    cout << endl;
    cout << "  怎么理解这些数字？" << endl;
    cout << "  1. 普通函数可内联时，编译器不仅省掉调用，还可能把循环向量化，" << endl;
    cout << "     所以和虚函数的差距会被放大。" << endl;
    cout << "  2. 强制不内联后，普通函数和虚函数的差距就小多了。" << endl;
    cout << "     单纯一次虚表间接调用，通常只多几纳秒。" << endl;
    cout << "  3. 虚函数真正的代价往往来自“不能内联、不能向量化、不能做常量传播”，" << endl;
    cout << "     而不是“查一次虚表”本身。" << endl;
    cout << endl;
    cout << "  虚函数不是坏东西，它是多态的基石。" << endl;
    cout << "  但在每帧调用几万次的热点路径里，可以考虑这些替代方案：" << endl;
    cout << "    - 用 final 标记不再被继承的类/函数，帮助编译器去虚化" << endl;
    cout << "    - 用 CRTP（奇异递归模板）做静态多态" << endl;
    cout << "    - 用 std::variant + std::visit 做封闭类型集合的多态" << endl;
    cout << "  普通业务逻辑，该用虚函数就用，别因小失大。" << endl;
}

// ================================================================
//  陷阱4：string 拼接的性能陷阱
// ================================================================
//
//  字符串拼接是日常操作，但很多人写得特别慢。
//
//  常见的错误写法：
//    string s;
//    for (int i = 0; i < 1000; ++i) {
//      s += "hello";  // 容量不够时会重新分配 + 搬移已有内容
//    }
//
//  注意：不是每次 += 都会重新分配。string 的容量通常按几何级数增长，
//  扩容只偶尔发生，累计搬移量是 O(n)，+= 的摊还复杂度是 O(1)。
//  但扩容次数多、字符串大时，提前 reserve 仍然能省下这些搬移。
//
//  更高效的写法：
//    - 先用 reserve 预分配足够空间
//    - 或者用 stringstream（但不一定更快）
//

void pitfall_string_concat() {
    cout << "\n========== 陷阱4：string 拼接 ==========" << endl;

    const int N = 500000;  // 拼接 50 万次，共约 3 MB
    const string piece = "hello ";  // 每次拼 6 字节

    // 方式1：直接 +=（不预分配）
    auto start = now();
    string s1;
    for (int i = 0; i < N; ++i) {
        s1 += piece;
    }
    auto end = now();
    double t1 = elapsed_ms(start, end);
    cout << "  【直接 += 不预分配】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t1 << " ms" << endl;
    cout << "    最终长度: " << s1.size() << endl;

    // 方式2：+= + reserve 预分配
    start = now();
    string s2;
    s2.reserve(static_cast<size_t>(N) * piece.size());  // 一次性分配够
    for (int i = 0; i < N; ++i) {
        s2 += piece;
    }
    end = now();
    double t2 = elapsed_ms(start, end);
    cout << "\n  【+= + reserve 预分配】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t2 << " ms" << endl;
    cout << "    最终长度: " << s2.size() << endl;

    // 方式3：stringstream
    start = now();
    stringstream ss;
    for (int i = 0; i < N; ++i) {
        ss << piece;
    }
    string s3 = ss.str();  // 注意：str() 会再复制一份完整字符串
    end = now();
    double t3 = elapsed_ms(start, end);
    cout << "\n  【stringstream】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t3 << " ms" << endl;
    cout << "    最终长度: " << s3.size() << endl;

    cout << "\n  结论：" << endl;
    cout << "  - 知道最终大小时，提前 reserve 通常最快" << endl;
    cout << "  - stringstream 适合混合类型拼接，但 str() 还会复制一次，" << endl;
    cout << "    不一定比 += 快" << endl;
    cout << "  - 直接 += 通常是摊还 O(1)：容量按几何级数增长，" << endl;
    cout << "    不是每次都重新分配" << endl;
    cout << "  - 拼接次数特别多、字符串特别大时，reserve 能减少扩容次数，" << endl;
    cout << "    也能降低峰值内存" << endl;
}

// ================================================================
//  陷阱5：vector 频繁扩容
// ================================================================
//
//  vector 的内存是连续的，当容量不够时，
//  它会：
//    1. 申请一块更大的内存（通常按 1.5 倍或 2 倍增长，具体由实现决定）
//    2. 把所有元素移动/拷贝过去
//    3. 释放旧内存
//
//  如果 push_back 很多次，会触发多次扩容。
//  但注意：几何增长保证了总搬移量是 O(n)，
//  push_back 的摊还复杂度是 O(1)，不是 O(n log n)。
//  扩容的真正代价在于：偶尔一次的大规模搬移、额外的峰值内存，
//  以及对“元素地址稳定”这一假设的破坏（扩容后旧指针/迭代器失效）。
//
//  解决方案：元素数量已知或能估算时，reserve 预分配。
//

void pitfall_vector_reserve() {
    cout << "\n========== 陷阱5：vector 频繁扩容 ==========" << endl;

    const int N = 20000000;  // 两千万个元素

    // 不预分配
    auto start = now();
    vector<int> v1;
    for (int i = 0; i < N; ++i) {
        v1.push_back(i);
    }
    auto end = now();
    double t1 = elapsed_ms(start, end);
    sink(v1.capacity());
    cout << "  【不预分配，直接 push_back】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t1 << " ms" << endl;
    cout << "    实际容量: " << v1.capacity() << endl;

    // 预分配
    start = now();
    vector<int> v2;
    v2.reserve(N);  // 一次性申请好
    for (int i = 0; i < N; ++i) {
        v2.push_back(i);
    }
    end = now();
    double t2 = elapsed_ms(start, end);
    sink(v2.capacity());
    cout << "\n  【reserve 预分配】" << endl;
    cout << "    耗时: " << fixed << setprecision(3) << t2 << " ms" << endl;
    cout << "    实际容量: " << v2.capacity() << endl;

    cout << "\n  差距通常不会特别夸张（int 很小，搬移基本就是 memcpy），" << endl;
    cout << "  但元素换成大对象后，扩容时的搬移成本会明显上升。" << endl;
    cout << "  reserve 省掉的是“扩容 + 搬移”，push_back 本身还是照常执行。" << endl;
    cout << "  不预分配还可能带来更高的峰值内存和更多的内存碎片。" << endl;
    cout << endl;
    cout << "  经验：元素数量已知，或能估个上限，就 reserve 一下；" << endl;
    cout << "       如果只是随便估一个很大的数，反而会浪费内存。" << endl;
}

// ================================================================
//  陷阱6：不必要的 shared_ptr
// ================================================================
//
//  shared_ptr 是神器，但不是免费的：
//  - 引用计数是原子操作（多线程安全，但慢）
//  - 拷贝 shared_ptr 要做原子递增
//  - 释放要做原子递减，归零还要调析构
//  - 控制块有额外内存开销
//
//  如果不需要共享所有权，就用 unique_ptr，甚至裸指针。
//
//  注意：这个例子中要区分"拷贝 shared_ptr"和"通过引用访问"。
//  真正慢的是频繁拷贝 shared_ptr。
//

// 用 noinline 工厂函数强制发生真实的堆分配，
// 防止编译器把 make_unique/make_shared 整个优化掉。
BENCH_NOINLINE unique_ptr<int> make_unique_int(int v) {
    return make_unique<int>(v);
}

BENCH_NOINLINE shared_ptr<int> make_shared_int(int v) {
    return make_shared<int>(v);
}

// 按值接收 shared_ptr：每次调用都会拷贝一次，引用计数 +1，函数结束时 -1。
BENCH_NOINLINE long long read_shared_by_value(shared_ptr<int> p) {
    return *p;
}

// 按 const 引用接收：不拷贝，不碰引用计数。
BENCH_NOINLINE long long read_shared_by_ref(const shared_ptr<int>& p) {
    return *p;
}

// 按值接收 unique_ptr：只能移动，不发生引用计数操作。
BENCH_NOINLINE long long read_unique_by_value(unique_ptr<int> p) {
    return *p;
}

void pitfall_shared_ptr() {
    cout << "\n========== 陷阱6：不必要的 shared_ptr ==========" << endl;

    const int N = 3000000;  // 三百万次

    // unique_ptr：构造（一次堆分配）+ 移动 + 访问
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_unique_by_value(make_unique_int(i));
    }
    auto end = now();
    double t_unique = elapsed_ms(start, end);
    sink(sum);
    cout << "  【unique_ptr 构造 + 移动 + 访问】" << endl;
    cout << "    三百万次耗时: " << fixed << setprecision(3)
         << t_unique << " ms" << endl;

    // shared_ptr：构造（控制块 + 原子计数初始化）+ 移动 + 访问
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_shared_by_value(make_shared_int(i));
    }
    end = now();
    double t_shared = elapsed_ms(start, end);
    sink(sum);
    cout << "\n  【shared_ptr 构造 + 移动 + 访问】" << endl;
    cout << "    三百万次耗时: " << fixed << setprecision(3)
         << t_shared << " ms" << endl;

    // shared_ptr 拷贝（原子递增 + 原子递减）
    shared_ptr<int> sp = make_shared_int(42);
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_shared_by_value(sp);  // 拷贝一次，引用计数 +1/-1
    }
    end = now();
    double t_copy = elapsed_ms(start, end);
    sink(sum);
    cout << "\n  【shared_ptr 按值传参（拷贝引用计数）】" << endl;
    cout << "    三百万次耗时: " << fixed << setprecision(3)
         << t_copy << " ms" << endl;

    // shared_ptr 只读引用（不碰引用计数）
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += read_shared_by_ref(sp);  // 不拷贝
    }
    end = now();
    double t_ref = elapsed_ms(start, end);
    sink(sum);
    cout << "\n  【shared_ptr 按 const 引用传参（不拷贝）】" << endl;
    cout << "    三百万次耗时: " << fixed << setprecision(3)
         << t_ref << " ms" << endl;

    if (t_unique > 0.0) {
        cout << "\n  shared_ptr 构造 / unique_ptr 构造 = "
             << fixed << setprecision(2) << (t_shared / t_unique) << " 倍"
             << endl;
    }
    if (t_ref > 0.0) {
        cout << "  shared_ptr 按值 / 按 const 引用 = "
             << fixed << setprecision(2) << (t_copy / t_ref) << " 倍"
             << endl;
    }

    cout << "\n  解释：" << endl;
    cout << "  - unique_ptr 对象本身只有一个指针那么大，没有引用计数；" << endl;
    cout << "    但 make_unique 依然会做一次堆分配（本例测的就是完整构造）。" << endl;
    cout << "  - shared_ptr 需要控制块；make_shared 通常把对象和控制块放在一次分配里。" << endl;
    cout << "  - shared_ptr 的拷贝要原子递增引用计数，析构要原子递减；" << endl;
    cout << "    单线程下这也有成本，多线程下还会产生缓存行竞争。" << endl;
    cout << "  - 引用计数通常是 lock-free 的，但“lock-free”不等于“免费”。" << endl;
    cout << endl;
    cout << "  经验法则：" << endl;
    cout << "    默认用 unique_ptr" << endl;
    cout << "    真的需要共享所有权，才用 shared_ptr" << endl;
    cout << "    只读访问传 const shared_ptr&；" << endl;
    cout << "    如果函数要保存一份所有权，再按值传参并 std::move" << endl;
}

// ================================================================
//  主函数
// ================================================================

int main() {
    cout << "================================================================" << endl;
    cout << "  常见性能陷阱 —— 那些偷偷吃掉性能的小细节" << endl;
    cout << "================================================================" << endl;
    cout << "\n  提示：建议用 Release 模式编译（-O2 或更高），" << endl;
    cout << "       优化级别不同，结果可能差异很大。" << endl;
    cout << "       本文件的基准测试都加了 noinline 和 volatile 汇聚，" << endl;
    cout << "       尽量防止编译器把被测代码整个优化掉。" << endl;
    cout << "       但绝对时间仍取决于机器，重点看数量级和相对差距。" << endl;

    pitfall_copy();              // 陷阱1：对象拷贝
    pitfall_implicit_conversion(); // 陷阱2：隐式类型转换
    pitfall_virtual();           // 陷阱3：虚函数
    pitfall_string_concat();     // 陷阱4：字符串拼接
    pitfall_vector_reserve();    // 陷阱5：vector 扩容
    pitfall_shared_ptr();        // 陷阱6：shared_ptr

    cout << "\n  【说明】以上均为单次计时，只用于演示趋势；" << endl;
    cout << "  真实基准测试应重复多轮、取中位数并先预热（见 tutorial.md）。" << endl;

    cout << "\n================================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  徒弟啊，今天讲的这六个陷阱，" << endl;
    cout << "  都是师傅当年踩过的坑，一个一个用血泪换来的教训。" << endl;
    cout << endl;
    cout << "  你可能会说：'这点性能差距算什么，无所谓。'" << endl;
    cout << "  师傅告诉你：" << endl;
    cout << "  单个来看，每个陷阱可能只慢一点点，" << endl;
    cout << "  但积少成多，一个项目里几十个、几百个这样的地方，" << endl;
    cout << "  加起来就是几倍的性能差距。" << endl;
    cout << endl;
    cout << "  更重要的是，写出高效的代码，应该成为你的习惯。" << endl;
    cout << "  不是为了抠那几毫秒，而是因为：" << endl;
    cout << "    - 好习惯写出的代码，天然就是快的" << endl;
    cout << "    - 坏习惯写多了，到时候想优化都找不到地方" << endl;
    cout << "    - 同样的功能，有人写出来快如闪电，有人写出来慢如蜗牛" << endl;
    cout << "    - 这就是普通程序员和优秀程序员的差距" << endl;
    cout << endl;
    cout << "  记住这几条铁律：" << endl;
    cout << "    1. 大对象传 const&，别传值" << endl;
    cout << "    2. 知道大小就 reserve，别让容器偷偷扩容" << endl;
    cout << "    3. 默认用 unique_ptr，shared_ptr 按需使用" << endl;
    cout << "    4. 字符串参数用 string_view（C++17）" << endl;
    cout << "    5. 热点代码里，虚函数和动态分派要三思" << endl;
    cout << "    6. 警惕隐式类型转换和临时对象" << endl;
    cout << endl;
    cout << "  最后送你一句话：" << endl;
    cout << "  「性能优化不是一蹴而就的，是一点一滴积累的。」" << endl;

    cout << "\n================================================================" << endl;
    cout << "  小练习" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  1. 【代码审查】找出下面代码中的性能问题：" << endl;
    cout << "     ```" << endl;
    cout << "     string concat(vector<string> words) {" << endl;
    cout << "         string result;" << endl;
    cout << "         for (auto w : words) {" << endl;
    cout << "             result = result + w + \" \";" << endl;
    cout << "         }" << endl;
    cout << "         return result;" << endl;
    cout << "     }" << endl;
    cout << "     ```" << endl;
    cout << "     （提示：有至少 4 个可以优化的地方）" << endl;
    cout << endl;
    cout << "  2. 【动手实验】把本文件的所有实验都跑一遍，" << endl;
    cout << "     记录下你的机器上的性能数据。" << endl;
    cout << "     再试试用不同优化级别编译（Debug vs Release），" << endl;
    cout << "     看看结果有什么变化。" << endl;
    cout << endl;
    cout << "  3. 【思考】为什么 string 的 += 操作" << endl;
    cout << "     即使不 reserve 也不算太慢？" << endl;
    cout << "     （提示：vector 的扩容策略是指数级的）" << endl;
    cout << "     那为什么还是建议 reserve？" << endl;
    cout << endl;
    cout << "  4. 【进阶】在你自己的代码里找一找，" << endl;
    cout << "     有没有踩到这些陷阱？" << endl;
    cout << "     试着改一改，测测前后性能差异。" << endl;
    cout << endl;

    return 0;
}
