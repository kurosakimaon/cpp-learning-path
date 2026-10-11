// ================================================================
//  cpp20_overview.cpp —— C++20/23 速览
//  知道有这么回事，大概长啥样
// ================================================================
//
//  注意：本文件需要 C++20/23 支持，老编译器可能编译不过。
//  主要用于了解特性概貌，不要求现在就掌握。
//  如果你的编译器不支持 C++20/23，可以先看代码和注释，
//  等以后升级了编译器再跑。
//
//  徒弟啊，C++ 这门语言一直在演进。
//  我们学了 C++11、14、17，那 C++20 和 C++23 呢？
//
//  C++20 可以说是继 C++11 之后又一个"大版本"，
//  加入了很多重量级特性。
//  C++23 则是在 C++20 基础上的完善和补充。
//
//  今天师傅就带你快速浏览一下 C++20/23 的主要特性。
//  不要求你现在就掌握，知道有这么回事、大概长啥样就行。
//  等你以后工作中遇到了，再深入学习也不迟。
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ranges>         // C++20 范围库
#include <concepts>       // C++20 概念
//#include <expected>      // C++23 std::expected（注意：有些编译器可能还不支持）
//#include <print>         // C++23 std::print（注意：支持度有限）

using namespace std;

// ================================================================
//  一、C++20 四大特性速览
// ================================================================
//
//  C++20 有四个"四大金刚"级别的特性：
//    1. Concepts（概念）
//    2. Ranges（范围库）
//    3. Coroutines（协程）
//    4. Modules（模块）
//
//  每一个都值得用一整章来讲，
//  这里我们只做"扫盲级"的介绍。
//

// ----------------------------------------------------------------
//  1. Concepts（概念）—— 约束模板参数
// ----------------------------------------------------------------
//
//  【解决的问题】
//  模板太灵活了，什么类型都能往里塞，
//  结果出错的时候，报错信息又长又难懂，
//  根本看不懂哪里错了。
//
//  【Concepts 做了什么】
//  给模板参数加上"约束"，明确告诉编译器和用户：
//  "这个模板只接受满足某些条件的类型"。
//
//  不满足条件的类型，编译器会直接给出清晰的错误提示，
//  而不是吐出几十行的模板实例化错误。
//
//  【和 SFINAE 的关系】
//  C++17 及之前，也能约束模板参数（用 SFINAE 技巧），
//  但写起来又丑又难懂，简直是玄学。
//  Concepts 把这件事变得优雅、直观。
//

// 定义一个概念：必须是可加的（支持 + 运算符）
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

// 用概念约束模板参数
// 只有满足 Addable 概念的类型，才能用这个函数
template<Addable T>
T add(T a, T b) {
    return a + b;
}

// 另一种写法：用 requires 子句
// 效果和上面一样，只是风格不同
template<typename T>
requires Addable<T>
T add2(T a, T b) {
    return a + b;
}

// 标准库里已经定义了很多常用概念
// 比如：std::integral（整型）、std::floating_point（浮点型）
//       std::ranges::range（范围）、std::copyable（可拷贝）...
template<std::integral T>   // 只接受整型
T square(T x) {
    return x * x;
}

void demo_concepts() {
    cout << "\n======== C++20 特性1：Concepts（概念） ========" << endl;
    cout << endl;
    cout << "  什么是 Concepts？" << endl;
    cout << "  给模板参数加上约束，明确指定\"这个模板能接受什么类型\"。" << endl;
    cout << "  比老式的 SFINAE 友好太多了！" << endl;
    cout << endl;

    // int 满足 Addable，没问题
    int sum = add(1, 2);
    cout << "  add(1, 2) = " << sum << endl;

    // string 也满足 Addable，没问题
    string s = add(string("hello "), string("world"));
    cout << "  add(\"hello \", \"world\") = " << s << endl;

    // 试试 square（只接受整型）
    cout << "  square(5) = " << square(5) << endl;

    // 如果你传一个不满足约束的类型，比如：
    //   square(3.14);  // 编译错误！
    // 编译器会给出清晰的提示：
    //   "约束未满足：std::integral<double> 为 false"
    // 而不是一堆看不懂的模板错误

    cout << endl;
    cout << "  为什么好？" << endl;
    cout << "    - 模板接口更清晰：一看就知道接受什么类型" << endl;
    cout << "    - 错误信息更友好：直接告诉你哪里不满足" << endl;
    cout << "    - 代码更易读：约束就是文档" << endl;
}

// ----------------------------------------------------------------
//  2. Ranges（范围库）—— 更优雅的算法
// ----------------------------------------------------------------
//
//  【解决的问题】
//  STL 算法（sort、find、transform...）都需要传两个迭代器：
//    sort(v.begin(), v.end());
//  每次都要写 begin 和 end，有点啰嗦。
//  而且多个算法组合的时候，要写好多临时变量。
//
//  【Ranges 做了什么】
//  引入了"范围"（Range）的概念——
//  一个范围就是"可以迭代的东西"（比如 vector、string、数组）。
//
//  主要好处：
//    1. 不用写 begin/end 了：ranges::sort(v);
//    2. 管道式写法：数据像流水一样经过一道道处理
//    3. 惰性求值：直到需要结果的时候才计算，效率高
//

void demo_ranges() {
    cout << "\n======== C++20 特性2：Ranges（范围库） ========" << endl;
    cout << endl;
    cout << "  什么是 Ranges？" << endl;
    cout << "  更优雅的算法库，支持管道式写法，代码更易读。" << endl;
    cout << endl;

    vector<int> nums = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    // ---- 传统写法（C++17）----
    cout << "  【传统写法】找偶数、平方、排序：" << endl;
    vector<int> evens;
    for (int n : nums) {
        if (n % 2 == 0) evens.push_back(n * n);
    }
    sort(evens.begin(), evens.end());
    cout << "    结果: ";
    for (int x : evens) cout << x << " ";
    cout << endl;

    // ---- Ranges 写法（C++20）----
    // 管道式：数据从左往右流，经过一道道"视图"
    // views::filter  —— 筛选
    // views::transform —— 转换
    // views::sort    —— 排序（用 ranges::sort）
    cout << "\n  【Ranges 管道式写法】找偶数、平方、排序：" << endl;

    auto result = nums
                | views::filter([](int n) { return n % 2 == 0; })
                | views::transform([](int n) { return n * n; });

    // 注意：views 是惰性的，上面这行还没真正计算
    // 我们把它放到 vector 里才会真正计算
    vector<int> result_vec(result.begin(), result.end());
    ranges::sort(result_vec);

    cout << "    结果: ";
    for (int x : result_vec) cout << x << " ";
    cout << endl;

    // ---- ranges 算法直接用 ----
    cout << "\n  【ranges::sort】直接对容器排序，不用写 begin/end：" << endl;
    vector<int> v = {3, 1, 4, 1, 5, 9};
    ranges::sort(v);  // 是不是简洁多了？
    cout << "    排序后: ";
    for (int x : v) cout << x << " ";
    cout << endl;

    cout << endl;
    cout << "  为什么好？" << endl;
    cout << "    - 代码更简洁：不用写 begin/end" << endl;
    cout << "    - 可读性更强：管道式写法，数据流向一目了然" << endl;
    cout << "    - 惰性求值：视图不立即计算，效率更高" << endl;
    cout << "    - 组合更灵活：各种 view 可以随意组合" << endl;
}

// ----------------------------------------------------------------
//  3. Coroutines（协程）—— 异步编程新范式
// ----------------------------------------------------------------
//
//  【解决的问题】
//  异步编程很麻烦：回调地狱、状态机复杂、代码难读。
//  协程让异步代码写起来像同步代码一样直观。
//
//  【协程是什么】
//  可以"暂停"和"恢复"执行的函数。
//  遇到需要等待的地方（比如 I/O、定时器），
//  就暂停执行，把 CPU 让出去；
//  等条件满足了，再从暂停的地方继续执行。
//
//  【三个关键字】
//    co_await  —— 等待一个异步操作完成
//    co_yield  —— 产出一个值并暂停（用于生成器）
//    co_return —— 返回最终结果并结束
//
//  注意：C++20 的协程是"底层机制"，
//  标准库只提供了最基础的支持，
//  真正好用的协程库（比如 std::generator）
//  要到 C++23 才有。
//  实际项目中，很多人用 cppcoro 等第三方库。
//

// 一个简单的协程例子：生成器（用 co_yield）
// 注意：这需要 C++23 的 std::generator，或者第三方库
// 为了让代码能编译（如果编译器不支持的话），
// 我们这里只做概念展示，用注释说明。

void demo_coroutines() {
    cout << "\n======== C++20 特性3：Coroutines（协程） ========" << endl;
    cout << endl;
    cout << "  什么是协程？" << endl;
    cout << "  可以暂停和恢复的函数，让异步代码写起来像同步代码。" << endl;
    cout << endl;

    cout << "  三个核心关键字：" << endl;
    cout << "    co_await  —— 等待异步操作完成" << endl;
    cout << "    co_yield  —— 产出一个值并暂停（生成器）" << endl;
    cout << "    co_return —— 返回最终结果" << endl;
    cout << endl;

    cout << "  示例（生成器，伪代码）：" << endl;
    cout << "    generator<int> fibonacci(int n) {" << endl;
    cout << "        int a = 0, b = 1;" << endl;
    cout << "        for (int i = 0; i < n; ++i) {" << endl;
    cout << "            co_yield a;     // 产出 a，然后暂停" << endl;
    cout << "            int c = a + b;" << endl;
    cout << "            a = b; b = c;" << endl;
    cout << "        }" << endl;
    cout << "    }" << endl;
    cout << endl;
    cout << "    // 使用：每次调用 next() 就恢复执行，拿到下一个值" << endl;
    cout << "    auto fib = fibonacci(5);" << endl;
    cout << "    // 产出: 0, 1, 1, 2, 3" << endl;
    cout << endl;

    cout << "  为什么好？" << endl;
    cout << "    - 异步代码更直观：写起来像同步代码" << endl;
    cout << "    - 避免回调地狱：不用一层层嵌套回调" << endl;
    cout << "    - 生成器模式：方便写出惰性计算的序列" << endl;
    cout << endl;
    cout << "  注意：C++20 协程是比较底层的特性，" << endl;
    cout << "  标准库的高层封装要到 C++23/26 才会完善。" << endl;
    cout << "  现在了解一下概念就行，不用急着深入。" << endl;
}

// ----------------------------------------------------------------
//  4. Modules（模块）—— 替代 #include
// ----------------------------------------------------------------
//
//  【解决的问题】
//  #include 有几个大问题：
//    1. 编译慢：每个源文件都要重新展开头文件
//    2. 容易出错：宏定义、头文件顺序依赖
//    3. 不模块化：头文件什么都暴露，没有"公有/私有"之分
//
//  【Modules 做了什么】
//  用 module 替代 #include，成为 C++ 新的模块化机制。
//
//  好处：
//    1. 编译快：模块只编译一次，结果可以复用
//    2. 更干净：没有宏污染，不用顾虑头文件顺序
//    3. 真正的模块化：可以明确导出哪些、不导出哪些
//
//  注意：模块的语法和构建方式比较新，
//  不同编译器的支持程度和使用方式有差异。
//  这里只做概念性介绍。
//

void demo_modules() {
    cout << "\n======== C++20 特性4：Modules（模块） ========" << endl;
    cout << endl;
    cout << "  什么是 Modules？" << endl;
    cout << "  替代 #include 的新模块化机制，编译更快、更干净。" << endl;
    cout << endl;

    cout << "  老式写法（#include）：" << endl;
    cout << "    #include <iostream>" << endl;
    cout << "    #include \"my_header.h\"" << endl;
    cout << "    // 问题：每个源文件都要重新展开，编译慢" << endl;
    cout << endl;

    cout << "  新式写法（模块）：" << endl;
    cout << "    import <iostream>;    // 导入标准库模块" << endl;
    cout << "    import my_module;     // 导入自定义模块" << endl;
    cout << endl;

    cout << "  定义模块（my_module.cpp）：" << endl;
    cout << "    export module my_module;  // 声明这是模块，且导出" << endl;
    cout << "    " << endl;
    cout << "    export void hello() {     // export 表示这个函数要导出" << endl;
    cout << "        // ..." << endl;
    cout << "    }" << endl;
    cout << "    " << endl;
    cout << "    void helper() {           // 没有 export，外部不可见" << endl;
    cout << "        // ..." << endl;
    cout << "    }" << endl;
    cout << endl;

    cout << "  为什么好？" << endl;
    cout << "    - 编译更快：模块只编译一次" << endl;
    cout << "    - 更干净：没有宏污染、头文件顺序问题" << endl;
    cout << "    - 真正的封装：可以控制哪些导出、哪些不导出" << endl;
    cout << endl;
    cout << "  注意：模块的构建系统支持还在完善中，" << endl;
    cout << "  不同编译器的用法略有差异。" << endl;
    cout << "  新项目可以尝试，老项目慢慢来。" << endl;
}

// ================================================================
//  二、C++23 特性速览
// ================================================================
//
//  C++23 是一个"中小版本"，没有 C++20 那么多革命性的东西，
//  但也加了不少实用特性。
//
//  这里挑几个最值得关注的讲讲。
//

// ----------------------------------------------------------------
//  1. std::expected —— 错误处理新方式
// ----------------------------------------------------------------
//
//  【解决的问题】
//  函数返回错误有几种方式，各有各的问题：
//    - 返回错误码：不直观，容易忽略
//    - 抛异常：有性能开销，有些场合不能用
//    - 返回 pair<T, error_code>：丑，用起来麻烦
//
//  【std::expected 是什么】
//  类似 Rust 的 Result 类型。
//  函数要么返回"期望的结果"，要么返回"错误信息"。
//
//  注意：std::expected 在 C++23 中加入，
//  有些编译器可能还不完整支持。
//

void demo_expected() {
    cout << "\n======== C++23 特性1：std::expected ========" << endl;
    cout << endl;
    cout << "  什么是 std::expected？" << endl;
    cout << "  类似 Rust 的 Result，函数要么返回正常值，要么返回错误。" << endl;
    cout << "  错误处理更清晰、更类型安全。" << endl;
    cout << endl;

    cout << "  示例（伪代码）：" << endl;
    cout << "    std::expected<int, std::string> divide(int a, int b) {" << endl;
    cout << "        if (b == 0) {" << endl;
    cout << "            return std::unexpected(\"除数不能为零\");" << endl;
    cout << "        }" << endl;
    cout << "        return a / b;" << endl;
    cout << "    }" << endl;
    cout << endl;
    cout << "    // 使用" << endl;
    cout << "    auto result = divide(10, 2);" << endl;
    cout << "    if (result) {" << endl;
    cout << "        cout << \"结果: \" << *result << endl;" << endl;
    cout << "    } else {" << endl;
    cout << "        cout << \"错误: \" << result.error() << endl;" << endl;
    cout << "    }" << endl;
    cout << endl;

    cout << "  为什么好？" << endl;
    cout << "    - 错误处理更明确：函数可能返回什么错误，一看就知道" << endl;
    cout << "    - 类型安全：编译器帮你检查有没有处理错误" << endl;
    cout << "    - 比异常轻量：没有栈展开的开销" << endl;
    cout << "    - 比错误码直观：不会忘，也不会和正常值混淆" << endl;
}

// ----------------------------------------------------------------
//  2. std::print / std::println —— 格式化输出
// ----------------------------------------------------------------
//
//  【解决的问题】
//  C++ 的输出一直很尴尬：
//    - cout 虽然类型安全，但写起来啰嗦，格式控制麻烦
//    - printf 虽然简洁，但类型不安全，容易出 bug
//
//  C++20 加入了 std::format（格式化字符串），
//  C++23 又加了 std::print / println（直接输出），
//  终于有了"既简洁又安全"的输出方式。
//

void demo_print() {
    cout << "\n======== C++23 特性2：std::print / std::println ========" << endl;
    cout << endl;
    cout << "  什么是 std::print？" << endl;
    cout << "  终于有了既简洁又类型安全的格式化输出！" << endl;
    cout << "  结合了 printf 的简洁和 cout 的类型安全。" << endl;
    cout << endl;

    cout << "  老式写法（cout）：" << endl;
    cout << "    cout << \"Hello, \" << name << \"! You are \" << age << \" years old.\" << endl;" << endl;
    cout << endl;

    cout << "  老式写法（printf）：" << endl;
    cout << "    printf(\"Hello, %s! You are %d years old.\\n\", name.c_str(), age);" << endl;
    cout << "    // 问题：类型不匹配会出 bug，string 还要 .c_str()" << endl;
    cout << endl;

    cout << "  C++23 新式写法：" << endl;
    cout << "    std::println(\"Hello, {}! You are {} years old.\", name, age);" << endl;
    cout << "    // 既简洁，又类型安全，还支持各种格式控制" << endl;
    cout << endl;

    cout << "  更多格式控制：" << endl;
    cout << "    std::println(\"{:10} | {:>8}\", \"名称\", \"价格\");  // 对齐" << endl;
    cout << "    std::println(\"{:.2f}\", 3.14159);                 // 小数位数" << endl;
    cout << "    std::println(\"{:08x}\", 255);                    // 十六进制补零" << endl;
    cout << endl;

    cout << "  为什么好？" << endl;
    cout << "    - 简洁：像 printf 一样好写" << endl;
    cout << "    - 安全：像 cout 一样类型安全" << endl;
    cout << "    - 功能强：各种格式控制都支持" << endl;
    cout << "    - 位置参数：支持 {0} {1} 这样的位置参数" << endl;
    cout << endl;
    cout << "  注意：std::format/print 是 C++20/23 最实用的特性之一，" << endl;
    cout << "  强烈建议尽快用起来。" << endl;
}

// ----------------------------------------------------------------
//  3. std::mdspan —— 多维数组视图
// ----------------------------------------------------------------
//
//  【解决的问题】
//  C++ 里的多维数组一直不太好用：
//    - 原生二维数组：栈上，大小固定，传参麻烦
//    - vector of vector：不连续，性能差
//    - 自己用一维数组模拟：要手动算索引，容易出错
//
//  【std::mdspan 是什么】
//  多维数组的"视图"（view），
//  它不拥有数据，只是提供一种多维的访问方式。
//
//  你给它一块连续的内存，告诉它各维度的大小，
//  它就能像多维数组一样访问，而且零开销。
//

void demo_mdspan() {
    cout << "\n======== C++23 特性3：std::mdspan ========" << endl;
    cout << endl;
    cout << "  什么是 mdspan？" << endl;
    cout << "  多维数组视图，给一维数据提供多维访问方式，零开销。" << endl;
    cout << endl;

    cout << "  示例（伪代码）：" << endl;
    cout << "    // 一维数组存数据（连续内存，性能好）" << endl;
    cout << "    vector<int> data = {1,2,3, 4,5,6, 7,8,9};" << endl;
    cout << endl;
    cout << "    // 用 mdspan 把它看成 3x3 的二维数组" << endl;
    cout << "    std::mdspan<int, std::extents<size_t, 3, 3>> mat(data.data());" << endl;
    cout << endl;
    cout << "    // 像二维数组一样访问" << endl;
    cout << "    cout << mat[0][0];  // 1" << endl;
    cout << "    cout << mat[1][2];  // 6" << endl;
    cout << "    cout << mat[2][1];  // 8" << endl;
    cout << endl;

    cout << "  为什么好？" << endl;
    cout << "    - 性能好：底层是连续内存，缓存友好" << endl;
    cout << "    - 方便：用多维索引访问，不用手动算偏移" << endl;
    cout << "    - 灵活：可以是 2 维、3 维...任意维" << endl;
    cout << "    - 零开销：视图不拥有数据，没有额外成本" << endl;
    cout << endl;
    cout << "  适合：科学计算、图像处理、矩阵运算等场景。" << endl;
}

// ----------------------------------------------------------------
//  4. C++23 其他小改进
// ----------------------------------------------------------------

void demo_cpp23_others() {
    cout << "\n======== C++23 其他小改进 ========" << endl;
    cout << endl;

    cout << "  1. 【if consteval】" << endl;
    cout << "     判断当前是否在编译期求值，" << endl;
    cout << "     让 consteval 函数里可以有不同的编译期/运行期路径。" << endl;
    cout << endl;

    cout << "  2. 【std::string::contains()】" << endl;
    cout << "     终于有了 contains 方法！" << endl;
    cout << "     不用再写 s.find(...) != string::npos 了。" << endl;
    cout << "     （string_view 在 C++23 也有 contains）" << endl;
    cout << endl;

    cout << "  3. 【std::ranges::to】" << endl;
    cout << "     把 range 直接转换成容器，" << endl;
    cout << "     不用再手动构造 begin/end。" << endl;
    cout << "     例如：auto v = src | views::transform(...) | ranges::to<vector>();" << endl;
    cout << endl;

    cout << "  4. 【auto() / auto{}】" << endl;
    cout << "     用 auto 构造临时对象，" << endl;
    cout << "     不用写长长的类型名。" << endl;
    cout << endl;

    cout << "  5. 【更完善的 constexpr 支持】" << endl;
    cout << "     更多标准库算法和容器支持 constexpr，" << endl;
    cout << "     编译期计算能力越来越强。" << endl;
    cout << endl;

    cout << "  6. 【std::flat_map / std::flat_set】" << endl;
    cout << "     基于有序数组的 map/set，" << endl;
    cout << "     内存连续，缓存友好，查找快。" << endl;
    cout << "     （适合数据量不大但频繁查找的场景）" << endl;
}

// ================================================================
//  主函数
// ================================================================

int main() {
    cout << "================================================================" << endl;
    cout << "  C++20/23 速览 —— 知道有这么回事，大概长啥样" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  注意：本文件需要 C++20/23 支持。" << endl;
    cout << "  如果你的编译器比较老，可能编译不过。" << endl;
    cout << "  没关系，看代码和注释也能了解个大概。" << endl;
    cout << "  等以后升级了编译器，再亲手试试。" << endl;

    // ---- C++20 四大特性 ----
    demo_concepts();
    demo_ranges();
    demo_coroutines();
    demo_modules();

    // ---- C++23 特性 ----
    demo_expected();
    demo_print();
    demo_mdspan();
    demo_cpp23_others();

    cout << "\n================================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  徒弟啊，今天讲了 C++20 和 C++23 的主要特性。" << endl;
    cout << "  内容很多，你可能记不住，没关系。" << endl;
    cout << "  师傅的要求很简单：知道有这么回事，大概长啥样。" << endl;
    cout << endl;
    cout << "  为什么要了解新版本？" << endl;
    cout << "  1. 【保持竞争力】" << endl;
    cout << "     C++ 一直在发展，你不学，就会被甩开。" << endl;
    cout << "     等大家都在用 C++20 了，你还在写 C++98 风格，" << endl;
    cout << "     怎么跟别人竞争？" << endl;
    cout << endl;
    cout << "  2. 【写出更好的代码】" << endl;
    cout << "     每个新版本的特性，都是为了解决实际问题的。" << endl;
    cout << "     知道有这些工具，遇到问题的时候你才能想到用它。" << endl;
    cout << "     不然手里只有锤子，看什么都是钉子。" << endl;
    cout << endl;
    cout << "  3. 【看懂别人的代码】" << endl;
    cout << "     开源项目、公司的新项目，都在用新标准。" << endl;
    cout << "     你不学，连别人的代码都看不懂。" << endl;
    cout << endl;
    cout << "  怎么跟进 C++ 的演进？" << endl;
    cout << "  1. 关注 C++ 标准委员会的动向（ISO C++ 官网、邮件列表）" << endl;
    cout << "  2. 看 CppCon 等会议的视频（YouTube 上有，都是大佬）" << endl;
    cout << "  3. 关注几个技术博客（比如 cppreference.com、fluentcpp.com）" << endl;
    cout << "  4. 每年新特性出来的时候，花点时间过一遍" << endl;
    cout << "  5. 在自己的项目里尝试用（从风险小的特性开始）" << endl;
    cout << endl;
    cout << "  最后师傅说一句：" << endl;
    cout << "  语言特性是工具，思想才是根本。" << endl;
    cout << "  不要为了追新而追新，也不要固步自封。" << endl;
    cout << "  保持学习的心态，但也要理性选择。" << endl;
    cout << "  适合自己项目的，才是最好的。" << endl;

    cout << "\n================================================================" << endl;
    cout << "  小练习" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  1. 【调研】查一下你当前使用的编译器版本，" << endl;
    cout << "     看看它支持哪些 C++20/23 特性。" << endl;
    cout << "     （搜索 \"GCC C++20 support\" 或 \"MSVC C++23 support\"）" << endl;
    cout << endl;
    cout << "  2. 【动手】如果你的编译器支持 C++20，" << endl;
    cout << "     试着用 ranges 重写一个你以前写的算法。" << endl;
    cout << "     感受一下管道式写法和传统写法的区别。" << endl;
    cout << endl;
    cout << "  3. 【思考】Concepts 和 SFINAE 比起来，" << endl;
    cout << "     有哪些优势？（提示：可读性、错误信息、...）" << endl;
    cout << endl;
    cout << "  4. 【调研】std::expected 和异常比起来，" << endl;
    cout << "     各有什么优缺点？分别适合什么场景？" << endl;
    cout << endl;
    cout << "  5. 【拓展】去 cppreference.com 看看 C++20/23 的完整特性列表，" << endl;
    cout << "     找几个你觉得有意思的特性，深入了解一下。" << endl;
    cout << endl;

    return 0;
}
