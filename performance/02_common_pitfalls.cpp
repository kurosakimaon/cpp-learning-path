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
#include <sstream>
#include <memory>      // shared_ptr
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace std::chrono;

// ================================================================
//  计时辅助工具
// ================================================================

inline high_resolution_clock::time_point now() {
    return high_resolution_clock::now();
}

inline double elapsed_ms(high_resolution_clock::time_point s,
                         high_resolution_clock::time_point e) {
    return duration<double, milli>(e - s).count();
}

// 防止编译器把整个循环优化掉的技巧
// 用 volatile 告诉编译器：这个变量的值很重要，别优化掉
volatile long long g_volatile_sink = 0;

template<typename T>
inline void sink(T&& x) {
    g_volatile_sink = reinterpret_cast<long long&>(x);
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

// 大对象（模拟）
struct BigObject {
    vector<int> data;
    BigObject(int n = 10000) : data(n, 42) {}
};

// ❌ 反面教材：传值（会拷贝整个 vector）
BigObject process_by_value(BigObject obj) {
    obj.data[0] = 100;
    return obj;
}

// ✅ 正确做法：传 const 引用 + 返回值优化
BigObject process_by_ref(const BigObject& obj) {
    BigObject result = obj;  // 只在需要的时候拷贝一次
    result.data[0] = 100;
    return result;
}

void pitfall_copy() {
    cout << "\n========== 陷阱1：不必要的对象拷贝 ==========" << endl;

    const int N = 10000;  // 循环 10000 次，差异更明显

    BigObject obj;

    // 传值方式
    auto start = now();
    for (int i = 0; i < N; ++i) {
        BigObject result = process_by_value(obj);
        sink(result.data.size());
    }
    auto end = now();
    double t_value = elapsed_ms(start, end);
    cout << "  【传值方式】耗时: " << fixed << setprecision(2)
         << t_value << " ms" << endl;

    // 传引用方式
    start = now();
    for (int i = 0; i < N; ++i) {
        BigObject result = process_by_ref(obj);
        sink(result.data.size());
    }
    end = now();
    double t_ref = elapsed_ms(start, end);
    cout << "  【传引用方式】耗时: " << fixed << setprecision(2)
         << t_ref << " ms" << endl;

    cout << "\n  为什么传值慢？" << endl;
    cout << "  传值 = 实参 → 形参（拷贝一次） + 返回值 → 接收方（可能再拷贝）" << endl;
    cout << "  传引用 = 不拷贝，直接用原对象" << endl;
    cout << endl;
    cout << "  规则：如果函数不修改参数，就用 const T&。" << endl;
    cout << "       内置类型（int/double/char*）太小，传值反而快。" << endl;
    cout << "       一般来说，大于指针大小两倍的对象，传引用更划算。" << endl;
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
size_t string_length_std(const string& s) {
    return s.size();
}

// ✅ 正面：用 string_view（C++17），零拷贝
size_t string_length_sv(string_view s) {
    return s.size();
}

void pitfall_implicit_conversion() {
    cout << "\n========== 陷阱2：隐式类型转换 ==========" << endl;

    const int N = 1000000;
    const char* raw_str = "hello, world! this is a test string.";

    // 传 const char* 给 const string& 参数
    // → 会隐式构造一个临时 string 对象
    auto start = now();
    size_t total = 0;
    for (int i = 0; i < N; ++i) {
        total += string_length_std(raw_str);
    }
    auto end = now();
    double t_str = elapsed_ms(start, end);
    cout << "  【const string& 参数 + 传 C 字符串】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t_str << " ms" << endl;
    cout << "    （每次调用都会构造临时 string！）" << endl;
    sink(total);

    // 用 string_view
    start = now();
    total = 0;
    for (int i = 0; i < N; ++i) {
        total += string_length_sv(raw_str);
    }
    end = now();
    double t_sv = elapsed_ms(start, end);
    cout << "\n  【string_view 参数 + 传 C 字符串】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t_sv << " ms" << endl;
    cout << "    （零拷贝，只是指针 + 长度）" << endl;
    sink(total);

    if (t_str > 0) {
        cout << "\n  性能差距: " << fixed << setprecision(1)
             << (t_str / t_sv) << " 倍" << endl;
    }
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

void pitfall_virtual() {
    cout << "\n========== 陷阱3：虚函数的开销 ==========" << endl;

    const int N = 100000000;  // 一亿次调用

    // 普通函数调用（大概率被内联）
    NormalCalc normal;
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += normal.calc(i);
    }
    auto end = now();
    double t_normal = elapsed_ms(start, end);
    cout << "  【普通函数调用】" << endl;
    cout << "    一亿次耗时: " << fixed << setprecision(2)
         << t_normal << " ms" << endl;
    sink(sum);

    // 虚函数调用（几乎不可能内联）
    BaseCalc* virt = new DerivedCalc();
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += virt->calc(i);
    }
    end = now();
    double t_virt = elapsed_ms(start, end);
    cout << "\n  【虚函数调用】" << endl;
    cout << "    一亿次耗时: " << fixed << setprecision(2)
         << t_virt << " ms" << endl;
    sink(sum);
    delete virt;

    if (t_normal > 0) {
        cout << "\n  性能差距: " << fixed << setprecision(1)
             << (t_virt / t_normal) << " 倍" << endl;
    }
    cout << endl;
    cout << "  为什么差这么多？" << endl;
    cout << "  普通函数：编译器可能直接内联，" << endl;
    cout << "            甚至把整个循环优化成几条指令。" << endl;
    cout << "  虚函数：编译器不知道具体调哪个函数，" << endl;
    cout << "          只能老老实实查虚表、调用，不能内联。" << endl;
    cout << endl;
    cout << "  注意：这不是说虚函数不好，而是说" << endl;
    cout << "  在性能极端敏感的地方（比如每帧调用几万次的热点），" << endl;
    cout << "  要考虑虚函数的开销。" << endl;
    cout << "  普通业务逻辑，该用就用，别瞎操心。" << endl;
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
//      s += "hello";  // 每次可能都要重新分配内存 + 拷贝
//    }
//
//  更高效的写法：
//    - 先用 reserve 预分配足够空间
//    - 或者用 stringstream（但不一定更快）
//

void pitfall_string_concat() {
    cout << "\n========== 陷阱4：string 拼接 ==========" << endl;

    const int N = 10000;  // 拼接 10000 次
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
    cout << "    耗时: " << fixed << setprecision(2) << t1 << " ms" << endl;
    cout << "    最终长度: " << s1.size() << endl;

    // 方式2：+= + reserve 预分配
    start = now();
    string s2;
    s2.reserve(N * piece.size());  // 一次性分配够
    for (int i = 0; i < N; ++i) {
        s2 += piece;
    }
    end = now();
    double t2 = elapsed_ms(start, end);
    cout << "\n  【+= + reserve 预分配】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t2 << " ms" << endl;
    cout << "    最终长度: " << s2.size() << endl;

    // 方式3：stringstream
    start = now();
    stringstream ss;
    for (int i = 0; i < N; ++i) {
        ss << piece;
    }
    string s3 = ss.str();
    end = now();
    double t3 = elapsed_ms(start, end);
    cout << "\n  【stringstream】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t3 << " ms" << endl;
    cout << "    最终长度: " << s3.size() << endl;

    cout << "\n  结论：" << endl;
    cout << "  - 知道最终大小的话，reserve 一下最快" << endl;
    cout << "  - stringstream 适合混合类型拼接，不一定最快" << endl;
    cout << "  - 直接 += 其实也没那么慢，vector 会指数扩容" << endl;
    cout << "  - 但如果拼接次数特别多，reserve 还是值得的" << endl;
}

// ================================================================
//  陷阱5：vector 频繁扩容
// ================================================================
//
//  vector 的内存是连续的，当容量不够时，
//  它会：
//    1. 申请一块更大的内存（通常 1.5 倍或 2 倍）
//    2. 把所有元素拷贝/移动过去
//    3. 释放旧内存
//
//  如果 push_back 很多次，会触发多次扩容，
//  每次都要拷贝所有元素，总开销是 O(n log n)。
//
//  解决方案：reserve 预分配。
//

void pitfall_vector_reserve() {
    cout << "\n========== 陷阱5：vector 频繁扩容 ==========" << endl;

    const int N = 10000000;  // 一千万个元素

    // 不预分配
    auto start = now();
    vector<int> v1;
    for (int i = 0; i < N; ++i) {
        v1.push_back(i);
    }
    auto end = now();
    double t1 = elapsed_ms(start, end);
    cout << "  【不预分配，直接 push_back】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t1 << " ms" << endl;
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
    cout << "\n  【reserve 预分配】" << endl;
    cout << "    耗时: " << fixed << setprecision(2) << t2 << " ms" << endl;
    cout << "    实际容量: " << v2.capacity() << endl;

    cout << "\n  差距可能不太大（int 拷贝很快），" << endl;
    cout << "  但如果是大对象（string、自定义类），差距会很明显。" << endl;
    cout << "  而且不预分配会造成内存碎片。" << endl;
    cout << endl;
    cout << "  经验：知道元素数量的话，就 reserve 一下。" << endl;
    cout << "       一行代码的事，没坏处。" << endl;
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

void pitfall_shared_ptr() {
    cout << "\n========== 陷阱6：不必要的 shared_ptr ==========" << endl;

    const int N = 10000000;  // 一千万次

    // unique_ptr（只转移所有权，不涉及引用计数）
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        auto p = make_unique<int>(i);
        sum += *p;
    }
    auto end = now();
    double t_unique = elapsed_ms(start, end);
    cout << "  【unique_ptr 构造 + 访问】" << endl;
    cout << "    一千万次耗时: " << fixed << setprecision(2)
         << t_unique << " ms" << endl;
    sink(sum);

    // shared_ptr（引用计数的原子开销）
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        auto p = make_shared<int>(i);
        sum += *p;
    }
    end = now();
    double t_shared = elapsed_ms(start, end);
    cout << "\n  【shared_ptr 构造 + 访问】" << endl;
    cout << "    一千万次耗时: " << fixed << setprecision(2)
         << t_shared << " ms" << endl;
    sink(sum);

    // shared_ptr 拷贝（原子递增的开销）
    auto sp = make_shared<int>(42);
    start = now();
    sum = 0;
    for (int i = 0; i < N; ++i) {
        auto p2 = sp;  // 拷贝一次，引用计数 +1
        sum += *p2;
    }
    end = now();
    double t_shared_copy = elapsed_ms(start, end);
    cout << "\n  【shared_ptr 拷贝 + 访问】" << endl;
    cout << "    一千万次耗时: " << fixed << setprecision(2)
         << t_shared_copy << " ms" << endl;
    sink(sum);

    cout << "\n  解释：" << endl;
    cout << "  - unique_ptr：几乎和裸指针一样快，零额外开销" << endl;
    cout << "  - shared_ptr 构造：要分配控制块 + 初始化引用计数" << endl;
    cout << "  - shared_ptr 拷贝：原子递增引用计数（原子操作比普通操作慢很多）" << endl;
    cout << endl;
    cout << "  经验法则：" << endl;
    cout << "    默认用 unique_ptr" << endl;
    cout << "    真的需要共享所有权，才用 shared_ptr" << endl;
    cout << "    函数传参用 const shared_ptr&，避免拷贝" << endl;
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

    pitfall_copy();              // 陷阱1：对象拷贝
    pitfall_implicit_conversion(); // 陷阱2：隐式类型转换
    pitfall_virtual();           // 陷阱3：虚函数
    pitfall_string_concat();     // 陷阱4：字符串拼接
    pitfall_vector_reserve();    // 陷阱5：vector 扩容
    pitfall_shared_ptr();        // 陷阱6：shared_ptr

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
