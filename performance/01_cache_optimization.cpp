// ================================================================
//  01_cache_optimization.cpp —— 缓存与内存优化
//  理解 CPU 缓存，写出"缓存友好"的代码
// ================================================================
//
//  徒弟啊，性能优化的第一课，不是什么奇技淫巧，
//  而是理解你代码跑在什么样的硬件上。
//
//  很多人写 C++，脑子里只有"代码逻辑对不对"，
//  从不关心数据在内存里怎么放、CPU 怎么读。
//  结果就是：代码逻辑没问题，但跑起来慢得像蜗牛。
//
//  今天师傅就带你搞懂一个最核心的概念：CPU 缓存。
//  理解了缓存，你对性能的认识会上升一个台阶。
//

#include <iostream>
#include <chrono>      // 高精度计时
#include <vector>
#include <cstdlib>     // malloc/free
#include <iomanip>     // 格式化输出

using namespace std;
using namespace std::chrono;

// ================================================================
//  一、CPU 缓存简介
// ================================================================
//
//  你可能听说过 L1、L2、L3 缓存，但它们到底是什么？
//
//  CPU 里有一小块非常快的内存，叫"缓存"（Cache）。
//  它的速度比内存（RAM）快几十上百倍，但容量很小。
//
//  典型的层级（速度从快到慢，容量从小到大）：
//
//    CPU 寄存器  →  最快，几个字节，就在运算单元旁边
//       ↓
//    L1 缓存    →  非常快，几十 KB，每个核心独享
//       ↓  约 1ns
//    L2 缓存    →  比较快，几百 KB，每个核心独享
//       ↓  约 3-5ns
//    L3 缓存    →  还行，几 MB ~ 几十 MB，多核共享
//       ↓  约 10-20ns
//    内存 RAM   →  慢，几十 GB，全机共享
//       ↓  约 100ns
//
//  这差距有多大呢？
//  如果把 L1 缓存的访问时间比作 1 秒，
//  那访问内存就相当于等 100 秒——差了两个数量级！
//
//  所以，CPU 不会傻乎乎地每次都去内存读数据。
//  它会把常用的数据放到缓存里，下次直接从缓存读。
//  这就引出了两个重要概念：
//
//  【时间局部性】：刚访问过的数据，很快还会再访问
//     → CPU 会把它留在缓存里
//
//  【空间局部性】：访问了某个数据，很快会访问它旁边的数据
//     → CPU 会把周围"一整块"数据都搬进缓存
//
//  这"一整块"有个名字，叫【缓存行】（Cache Line）。
//  通常是 64 字节。也就是说，你只读 1 个字节，
//  CPU 会把它周围 64 字节都搬进缓存。
//
//  理解了这些，你就能写出"缓存友好"的代码了。
//

// 辅助函数：获取当前时间点（高精度）
inline high_resolution_clock::time_point now() {
    return high_resolution_clock::now();
}

// 辅助函数：计算耗时（毫秒）
inline double elapsed_ms(high_resolution_clock::time_point start,
                         high_resolution_clock::time_point end) {
    return duration<double, milli>(end - start).count();
}

// ================================================================
//  二、行优先 vs 列优先遍历（二维数组）
// ================================================================
//
//  这是最经典的缓存优化例子，没有之一。
//
//  C++ 的二维数组是【行优先】存储的：
//  arr[i][j] 旁边是 arr[i][j+1]，而不是 arr[i+1][j]
//
//  内存里长这样：
//    [第0行: 0,0 | 0,1 | 0,2 | ...] [第1行: 1,0 | 1,1 | ...] ...
//
//  所以如果你按"行优先"遍历（先变列，再变行），
//  每次访问的都是内存里相邻的元素 → 空间局部性好 → 缓存命中率高
//
//  反过来，按"列优先"遍历（先变行，再变列），
//  每次跳一行，内存里隔得很远 → 空间局部性差 → 缓存命中率低
//
//  理论说了不算，我们来实测一下！
//

const int MATRIX_SIZE = 1000;  // 1000x1000 的矩阵，够大了

// 行优先遍历（正确姿势）
double row_major_traversal(vector<vector<int>>& mat) {
    auto start = now();
    long long sum = 0;
    for (int i = 0; i < MATRIX_SIZE; ++i) {        // 外层：行
        for (int j = 0; j < MATRIX_SIZE; ++j) {    // 内层：列
            sum += mat[i][j];                       // 每次访问下一列（相邻内存）
        }
    }
    auto end = now();
    // 输出一下 sum，防止编译器把整个循环优化掉
    cout << "  求和结果: " << sum << "  (防止编译器优化)" << endl;
    return elapsed_ms(start, end);
}

// 列优先遍历（反面教材）
double col_major_traversal(vector<vector<int>>& mat) {
    auto start = now();
    long long sum = 0;
    for (int j = 0; j < MATRIX_SIZE; ++j) {        // 外层：列
        for (int i = 0; i < MATRIX_SIZE; ++i) {    // 内层：行
            sum += mat[i][j];                       // 每次访问下一行（跳一大段内存）
        }
    }
    auto end = now();
    cout << "  求和结果: " << sum << "  (防止编译器优化)" << endl;
    return elapsed_ms(start, end);
}

void demo_row_vs_col_major() {
    cout << "\n========== 实验1：行优先 vs 列优先遍历 ==========" << endl;

    // 初始化矩阵
    cout << "  初始化 " << MATRIX_SIZE << "x" << MATRIX_SIZE
         << " 矩阵..." << endl;
    vector<vector<int>> mat(MATRIX_SIZE, vector<int>(MATRIX_SIZE));
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            mat[i][j] = i * MATRIX_SIZE + j;

    // 行优先
    cout << "\n  【行优先遍历】（先变列，再变行）" << endl;
    double t_row = row_major_traversal(mat);
    cout << "  耗时: " << fixed << setprecision(2) << t_row << " ms" << endl;

    // 列优先
    cout << "\n  【列优先遍历】（先变行，再变列）" << endl;
    double t_col = col_major_traversal(mat);
    cout << "  耗时: " << fixed << setprecision(2) << t_col << " ms" << endl;

    // 对比
    cout << "\n  ---- 对比 ----" << endl;
    cout << "  行优先比列优先快: " << fixed << setprecision(1)
         << (t_col / t_row) << " 倍" << endl;
    cout << endl;
    cout << "  为什么差这么多？" << endl;
    cout << "  行优先：每次访问的是内存里相邻的元素，" << endl;
    cout << "          CPU 把一整行搬进缓存后，后面的访问都命中缓存。" << endl;
    cout << "  列优先：每次跳一行，内存里隔了几千字节，" << endl;
    cout << "          几乎每次都要重新从内存加载缓存行。" << endl;
}

// ================================================================
//  三、结构体对齐与填充（Struct Padding）
// ================================================================
//
//  你知道吗？结构体的大小，不一定等于各成员大小之和。
//  CPU 访问内存时，喜欢"对齐"的数据——地址是 4 的倍数、8 的倍数……
//  不对齐的话，访问速度会变慢，甚至某些架构直接报错。
//
//  所以编译器会自动在结构体成员之间插入"填充字节"（padding），
//  让每个成员都对齐到合适的地址上。
//
//  这就导致了：
//  - 结构体实际大小比你想象的大
//  - 如果成员顺序排得不好，会浪费很多空间
//  - 空间浪费 → 缓存能装的对象变少 → 缓存命中率下降
//
//  来看例子：
//

// 排列不好的结构体（小大交错，浪费空间）
struct BadStruct {
    char a;       // 1 字节
    double b;     // 8 字节
    char c;       // 1 字节
    int d;        // 4 字节
    // 猜猜这个结构体多大？
    // 答案：不是 1+8+1+4=14，而是 24 字节！
};

// 排列好的结构体（从大到小排，减少填充）
struct GoodStruct {
    double b;     // 8 字节（最大的放前面）
    int d;        // 4 字节
    char a;       // 1 字节
    char c;       // 1 字节
    // 大小：8+4+1+1=14，但因为对齐，实际是 16 字节
    // 比上面的 24 字节少了 1/3！
};

void demo_struct_padding() {
    cout << "\n========== 实验2：结构体对齐与填充 ==========" << endl;

    cout << "\n  【排列不好的结构体 BadStruct】" << endl;
    cout << "    成员: char(1) + double(8) + char(1) + int(4)" << endl;
    cout << "    你以为的大小: 14 字节" << endl;
    cout << "    实际大小: " << sizeof(BadStruct) << " 字节" << endl;

    cout << "\n  【排列好的结构体 GoodStruct】" << endl;
    cout << "    成员: double(8) + int(4) + char(1) + char(1)" << endl;
    cout << "    你以为的大小: 14 字节" << endl;
    cout << "    实际大小: " << sizeof(GoodStruct) << " 字节" << endl;

    cout << "\n  为什么会这样？" << endl;
    cout << "  编译器要保证每个成员都"对齐"：" << endl;
    cout << "    - char   对齐到 1 字节（随便放）" << endl;
    cout << "    - int    对齐到 4 字节（地址末两位是 00）" << endl;
    cout << "    - double 对齐到 8 字节（地址末三位是 000）" << endl;
    cout << "  不对齐怎么办？插空字节（padding）！" << endl;

    cout << "\n  BadStruct 的内存布局：" << endl;
    cout << "    [a][ 填 7 个空 ][      b      ][c][ 填 3 个空 ][   d   ]" << endl;
    cout << "     1      7           8         1      3          4" << endl;
    cout << "     = 24 字节" << endl;

    cout << "\n  GoodStruct 的内存布局：" << endl;
    cout << "    [      b      ][   d   ][a][c][ 填 2 个空 ]" << endl;
    cout << "          8           4      1  1      2" << endl;
    cout << "     = 16 字节" << endl;

    cout << "\n  省了多少？" << (sizeof(BadStruct) - sizeof(GoodStruct))
         << " 字节 / 每个对象" << endl;
    cout << "  如果有 100 万个对象，就省了 "
         << (sizeof(BadStruct) - sizeof(GoodStruct)) * 1000000 / 1024 / 1024
         << " MB！" << endl;
    cout << "  省空间 = 更多数据进缓存 = 更快！" << endl;
}

// ================================================================
//  四、数组 vs 链表的遍历性能
// ================================================================
//
//  数据结构课上老师说：
//  数组查找 O(1)，插入删除 O(n)
//  链表查找 O(n)，插入删除 O(1)
//
//  但实际跑起来，链表就算只做遍历，都比数组慢得多！
//  为什么？因为链表的节点在内存里是散的，
//  每个节点都要重新从内存加载 → 缓存几乎不命中。
//
//  而数组是连续的，遍历的时候缓存命中率极高。
//
//  我们来实测一下：
//

// 链表节点
struct ListNode {
    int value;
    ListNode* next;
};

void demo_array_vs_list() {
    cout << "\n========== 实验3：数组 vs 链表遍历 ==========" << endl;

    const int N = 1000000;  // 100 万个元素
    cout << "  元素个数: " << N << endl;

    // ---- 数组 ----
    cout << "\n  【数组 (vector)】" << endl;
    vector<int> arr(N);
    for (int i = 0; i < N; ++i) arr[i] = i;

    auto start = now();
    long long sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += arr[i];
    }
    auto end = now();
    cout << "    求和结果: " << sum << endl;
    cout << "    遍历耗时: " << fixed << setprecision(2)
         << elapsed_ms(start, end) << " ms" << endl;

    // ---- 链表 ----
    cout << "\n  【链表 (手写)】" << endl;
    ListNode* head = nullptr;
    ListNode* tail = nullptr;
    for (int i = 0; i < N; ++i) {
        ListNode* node = new ListNode{i, nullptr};
        if (!head) { head = node; tail = node; }
        else { tail->next = node; tail = node; }
    }

    start = now();
    sum = 0;
    ListNode* cur = head;
    while (cur) {
        sum += cur->value;
        cur = cur->next;
    }
    end = now();
    cout << "    求和结果: " << sum << endl;
    cout << "    遍历耗时: " << fixed << setprecision(2)
         << elapsed_ms(start, end) << " ms" << endl;

    // 清理链表
    cur = head;
    while (cur) {
        ListNode* next = cur->next;
        delete cur;
        cur = next;
    }

    cout << "\n  你会发现，链表遍历慢很多（可能 2-5 倍，甚至更多）。" << endl;
    cout << "  为什么？" << endl;
    cout << "    数组：内存连续，CPU 预取（prefetch）效果好，缓存命中率高。" << endl;
    cout << "    链表：节点散落在内存各处，next 指针一跳就是随机位置，" << endl;
    cout << "          几乎每次都缓存不命中，要等内存。" << endl;
    cout << endl;
    cout << "  结论：能用数组/vector，就别用链表。" << endl;
    cout << "       现代 CPU 上，连续内存 = 性能！" << endl;
}

// ================================================================
//  五、SOA vs AOS（结构体的两种布局）
// ================================================================
//
//  假设你有很多"人"的数据：姓名、年龄、工资。
//
//  常见的写法是这样的（AOS - Array of Structures）：
//    struct Person { string name; int age; double salary; };
//    vector<Person> people;
//
//  内存里长这样：
//    [name1][age1][salary1] [name2][age2][salary2] [name3]...
//
//  但如果你只需要处理"年龄"这一个字段呢？
//  你遍历数组的时候，name 和 salary 也会被一起加载进缓存，
//  但你根本不用它们 → 浪费缓存空间！
//
//  这时候有一种写法叫 SOA（Structure of Arrays）：
//    struct People {
//      vector<string> names;
//      vector<int> ages;
//      vector<double> salaries;
//    };
//
//  内存里长这样：
//    [name1][name2][name3]... [age1][age2][age3]... [salary1][salary2]...
//
//  每个字段是独立的数组，连续存储。
//  如果你只处理年龄，加载进缓存的全是年龄数据，
//  缓存利用率极高！
//

// AOS: Array of Structures（传统写法）
struct Particle_AOS {
    double x;   // 位置 x
    double y;   // 位置 y
    double vx;  // 速度 x
    double vy;  // 速度 y
    double mass;// 质量
    // 总共 5 个 double = 40 字节
};

// SOA: Structure of Arrays（数据导向设计）
struct Particles_SOA {
    vector<double> x;
    vector<double> y;
    vector<double> vx;
    vector<double> vy;
    vector<double> mass;
};

void demo_soa_vs_aos() {
    cout << "\n========== 实验4：AOS vs SOA ==========" << endl;

    const int N = 1000000;  // 100 万个粒子
    cout << "  粒子个数: " << N << endl;

    // ---- AOS ----
    cout << "\n  【AOS - Array of Structures】" << endl;
    vector<Particle_AOS> particles_aos(N);
    for (int i = 0; i < N; ++i) {
        particles_aos[i].x = i * 0.1;
        particles_aos[i].y = i * 0.2;
        particles_aos[i].vx = i * 0.01;
        particles_aos[i].vy = i * 0.02;
        particles_aos[i].mass = 1.0;
    }

    // 只更新位置（只用 x, y, vx, vy，不用 mass）
    auto start = now();
    for (int i = 0; i < N; ++i) {
        particles_aos[i].x += particles_aos[i].vx;
        particles_aos[i].y += particles_aos[i].vy;
    }
    auto end = now();
    cout << "    更新位置耗时: " << fixed << setprecision(2)
         << elapsed_ms(start, end) << " ms" << endl;

    // ---- SOA ----
    cout << "\n  【SOA - Structure of Arrays】" << endl;
    Particles_SOA particles_soa;
    particles_soa.x.resize(N);
    particles_soa.y.resize(N);
    particles_soa.vx.resize(N);
    particles_soa.vy.resize(N);
    particles_soa.mass.resize(N, 1.0);
    for (int i = 0; i < N; ++i) {
        particles_soa.x[i] = i * 0.1;
        particles_soa.y[i] = i * 0.2;
        particles_soa.vx[i] = i * 0.01;
        particles_soa.vy[i] = i * 0.02;
    }

    // 只更新位置
    start = now();
    for (int i = 0; i < N; ++i) {
        particles_soa.x[i] += particles_soa.vx[i];
        particles_soa.y[i] += particles_soa.vy[i];
    }
    end = now();
    cout << "    更新位置耗时: " << fixed << setprecision(2)
         << elapsed_ms(start, end) << " ms" << endl;

    cout << "\n  解释：" << endl;
    cout << "  AOS 的每个粒子 40 字节，你加载一个缓存行（64 字节），" << endl;
    cout << "  只能装 1 个多粒子的完整数据，而且 mass 字段你根本不用。" << endl;
    cout << "  SOA 的 x 数组全是 double，一个缓存行能装 8 个 x 值，" << endl;
    cout << "  全部都是你要用的数据，缓存利用率拉满！" << endl;
    cout << endl;
    cout << "  注意：SOA 不是银弹。" << endl;
    cout << "  - 如果你的访问模式是'每次都用所有字段'，AOS 可能更好。" << endl;
    cout << "  - 如果你的访问模式是'每次只用某几个字段'，SOA 优势明显。" << endl;
    cout << "  - 游戏引擎、物理模拟、数据处理……这些领域 SOA 用得很多。" << endl;
}

// ================================================================
//  主函数
// ================================================================

int main() {
    cout << "================================================================" << endl;
    cout << "  缓存与内存优化 —— 写出缓存友好的代码" << endl;
    cout << "================================================================" << endl;

    cout << "\n  说明：以下实验会测量真实运行时间。" << endl;
    cout << "  结果会受编译器优化级别、CPU 型号、系统负载影响。" << endl;
    cout << "  建议用 Release 模式（O2/O3 优化）编译，效果更明显。" << endl;

    // 实验1：行优先 vs 列优先
    demo_row_vs_col_major();

    // 实验2：结构体对齐
    demo_struct_padding();

    // 实验3：数组 vs 链表
    demo_array_vs_list();

    // 实验4：AOS vs SOA
    demo_soa_vs_aos();

    cout << "\n================================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  徒弟啊，今天这一课，师傅没教你什么新语法，" << endl;
    cout << "  但它的重要性，比 10 个语法特性加起来都大。" << endl;
    cout << endl;
    cout << "  很多人写了好几年 C++，都不知道缓存这回事。" << endl;
    cout << "  他们写的代码，逻辑没错，但跑起来就是慢。" << endl;
    cout << "  你今天懂了缓存，就已经超过了一半的 C++ 程序员。" << endl;
    cout << endl;
    cout << "  记住几个核心要点：" << endl;
    cout << "    1. 连续内存是性能之友。能用数组就别用链表。" << endl;
    cout << "    2. 遍历的时候，按内存顺序走（行优先）。" << endl;
    cout << "    3. 结构体成员按大小排序，减少 padding。" << endl;
    cout << "    4. 如果只用部分字段，考虑 SOA 布局。" << endl;
    cout << "    5. 性能瓶颈通常不在算法，而在内存访问模式。" << endl;
    cout << endl;
    cout << "  最后送你一句话：" << endl;
    cout << "  「写代码的时候，脑子里要有内存的样子。」" << endl;

    cout << "\n================================================================" << endl;
    cout << "  小练习" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  1. 【动手验证】把矩阵大小改成 2000 或 500，" << endl;
    cout << "     再测一次行优先和列优先的速度比。" << endl;
    cout << "     看看矩阵大小对性能差异有什么影响。" << endl;
    cout << endl;
    cout << "  2. 【结构体优化】下面这个结构体，怎么重排成员能让它最小？" << endl;
    cout << "     struct { char a; short b; int c; char d; double e; };" << endl;
    cout << "     优化前多大？优化后多大？用 sizeof 验证你的答案。" << endl;
    cout << endl;
    cout << "  3. 【思考】为什么 std::deque 比 std::list 快？" << endl;
    cout << "     （提示：deque 是分块连续的，不是完全链表）" << endl;
    cout << endl;
    cout << "  4. 【进阶】在你的项目里找一个热点循环，" << endl;
    cout << "     看看能不能通过调整数据布局让它变快。" << endl;
    cout << "     记得用计时工具测前后对比！" << endl;
    cout << endl;

    return 0;
}
