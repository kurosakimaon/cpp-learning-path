/**
 * @file parallel_algorithms.cpp
 * @brief C++17 并行算法（Parallel Algorithms）深入解析
 *
 * 知识点覆盖：
 * 1. 执行策略（execution policy）：seq / par / par_unseq 的语义区别
 * 2. 并行算法的使用：sort / for_each / reduce / transform 等
 * 3. 并行 vs 串行的性能对比与何时使用并行
 * 4. 注意事项：数据竞争、异常安全、迭代器要求
 * 5. 数值算法：reduce / transform_reduce / exclusive_scan / inclusive_scan
 * 6. 实际工程中的性能考量与调优建议
 *
 * 编译命令（需要 TBB 支持）:
 *   g++ -std=c++17 -O2 parallel_algorithms.cpp -o parallel_algorithms -ltbb
 *   (Windows MSVC: 不需要额外库，标准库自带并行支持)
 *
 * 如果没有 TBB，可以定义 NO_PARALLEL 宏来跳过并行演示部分
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>
#include <string>
#include <execution>
#include <cmath>
#include <mutex>
#include <atomic>

// 检查是否支持并行策略（简单检测）
#if defined(__cpp_lib_parallel_algorithm)
constexpr bool HAS_PARALLEL = true;
#else
constexpr bool HAS_PARALLEL = false;
#endif

// ============================================================
// 第一部分：执行策略详解
// ============================================================

namespace execution_policy {

// C++17 定义了三种标准执行策略：
// 1. std::execution::sequenced_policy (seq)
//    - 串行执行，元素按顺序处理
//    - 与普通算法行为完全一致
//    - 可以在元素访问之间有依赖关系
//
// 2. std::execution::parallel_policy (par)
//    - 并行执行，多个线程同时处理不同元素
//    - 元素处理顺序不确定
//    - 用户代码不能有数据竞争（需要自己保证线程安全）
//    - 异常会被捕获并调用 std::terminate（或传播给调用者，具体实现决定）
//
// 3. std::execution::parallel_unsequenced_policy (par_unseq)
//    - 并行 + 向量化（SIMD）
//    - 最宽松的策略，允许最大程度的优化
//    - 元素处理不仅可以并行，还可以乱序、向量化
//    - 限制最严格：不能分配内存、不能加锁、不能调用非向量化安全的函数

void demo_policy_concepts() {
    std::cout << "===== 1. 执行策略概念 =====\n";

    std::cout << "std::execution::seq (sequenced_policy)\n";
    std::cout << "  - 串行执行，顺序确定\n";
    std::cout << "  - 等价于普通算法调用\n\n";

    std::cout << "std::execution::par (parallel_policy)\n";
    std::cout << "  - 多线程并行执行\n";
    std::cout << "  - 顺序不确定，用户保证线程安全\n\n";

    std::cout << "std::execution::par_unseq (parallel_unsequenced_policy)\n";
    std::cout << "  - 并行 + 向量化\n";
    std::cout << "  - 限制最严格：不能分配/加锁/调用非向量化函数\n";
    std::cout << "  - 性能最高，但约束最多\n";
}

} // namespace execution_policy


// ============================================================
// 第二部分：并行算法基础演示
// ============================================================

namespace basic_parallel {

// 工具函数：生成随机数据
std::vector<int> generate_random_data(size_t size, int seed = 42) {
    std::vector<int> data(size);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(1, 1000000);
    for (auto& x : data) x = dist(gen);
    return data;
}

// 工具函数：计时
template <typename Func>
auto measure_time(Func&& func, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "  " << name << ": " << duration.count() << " ms\n";
    return duration.count();
}

void demo_parallel_sort() {
    std::cout << "\n===== 2.1 并行排序 std::sort =====\n";

    const size_t N = 2'000'000;  // 200 万数据
    std::cout << "数据量: " << N << " 个 int\n\n";

    auto data1 = generate_random_data(N);
    auto data2 = data1;
    auto data3 = data1;

    // 串行排序
    auto t1 = measure_time([&]() {
        std::sort(std::execution::seq, data1.begin(), data1.end());
    }, "seq 排序");

    // 并行排序
    double t2 = 0;
    if constexpr (HAS_PARALLEL) {
        t2 = measure_time([&]() {
            std::sort(std::execution::par, data2.begin(), data2.end());
        }, "par 排序");

        // 验证结果一致
        bool same = (data1 == data2);
        std::cout << "  结果一致性: " << std::boolalpha << same << "\n";

        if (t1 > 0 && t2 > 0) {
            std::cout << "  加速比: " << (t1 / t2) << "x\n";
        }
    } else {
        std::cout << "  (当前环境不支持并行算法，跳过 par 演示)\n";
    }
}

void demo_parallel_for_each() {
    std::cout << "\n===== 2.2 并行 for_each =====\n";

    const size_t N = 1'000'000;
    std::cout << "数据量: " << N << " 个 double\n\n";

    std::vector<double> data(N);
    std::iota(data.begin(), data.end(), 1.0);  // 1, 2, 3, ...

    // 串行：对每个元素做计算
    auto t1 = measure_time([&]() {
        std::for_each(std::execution::seq, data.begin(), data.end(),
            [](double& x) {
                x = std::sqrt(x) * std::sin(x) + std::cos(x * 0.5);
            });
    }, "seq for_each");

    if constexpr (HAS_PARALLEL) {
        std::vector<double> data2(N);
        std::iota(data2.begin(), data2.end(), 1.0);

        auto t2 = measure_time([&]() {
            std::for_each(std::execution::par, data2.begin(), data2.end(),
                [](double& x) {
                    x = std::sqrt(x) * std::sin(x) + std::cos(x * 0.5);
                });
        }, "par for_each");

        if (t1 > 0) {
            std::cout << "  加速比: " << (t1 / t2) << "x\n";
        }
    } else {
        std::cout << "  (当前环境不支持并行算法，跳过 par 演示)\n";
    }
}

} // namespace basic_parallel


// ============================================================
// 第三部分：数值并行算法（reduce / transform_reduce）
// ============================================================

namespace numeric_parallel {

// 3.1 std::reduce vs std::accumulate
// accumulate 是严格串行的，按顺序累加
// reduce 可以并行，但要求操作满足结合律（顺序不影响结果）

void demo_reduce() {
    std::cout << "\n===== 3.1 std::reduce 并行求和 =====\n";

    const size_t N = 10'000'000;
    std::cout << "数据量: " << N << " 个 double\n\n";

    std::vector<double> data(N);
    std::iota(data.begin(), data.end(), 1.0);

    // std::accumulate（串行）
    auto t1 = basic_parallel::measure_time([&]() {
        volatile double sum = std::accumulate(data.begin(), data.end(), 0.0);
        (void)sum;
    }, "accumulate (串行)");

    // std::reduce + seq
    auto t2 = basic_parallel::measure_time([&]() {
        volatile double sum = std::reduce(std::execution::seq, data.begin(), data.end(), 0.0);
        (void)sum;
    }, "reduce seq");

    if constexpr (HAS_PARALLEL) {
        auto t3 = basic_parallel::measure_time([&]() {
            volatile double sum = std::reduce(std::execution::par, data.begin(), data.end(), 0.0);
            (void)sum;
        }, "reduce par");

        if (t1 > 0) {
            std::cout << "  相对 accumulate 加速比: " << (t1 / t3) << "x\n";
        }
    } else {
        std::cout << "  (当前环境不支持并行算法，跳过 par 演示)\n";
    }

    // reduce 与 accumulate 的语义区别：
    // - accumulate 的操作是严格左结合的：(((init + a1) + a2) + ...)
    // - reduce 的操作顺序不确定，操作必须满足结合律
    // - 对于浮点数，顺序不同可能导致微小的精度差异
    double acc_sum = std::accumulate(data.begin(), data.end(), 0.0);
    double red_sum = std::reduce(std::execution::seq, data.begin(), data.end(), 0.0);
    std::cout << "  结果精度差异(seq): " << std::abs(acc_sum - red_sum)
              << " (seq 模式下通常完全一致)\n";
}

// 3.2 std::transform_reduce：先转换再归约
// 这是 map-reduce 的标准库版本

void demo_transform_reduce() {
    std::cout << "\n===== 3.2 std::transform_reduce =====\n";

    const size_t N = 1'000'000;
    std::cout << "数据量: " << N << " 个 int\n\n";

    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 1);

    // 计算所有数的平方和
    // 串行方式：先 transform 再 accumulate
    auto t1 = basic_parallel::measure_time([&]() {
        long long sum = 0;
        for (int x : data) sum += static_cast<long long>(x) * x;
        volatile auto s = sum;
        (void)s;
    }, "普通循环平方和");

    // transform_reduce 方式
    auto t2 = basic_parallel::measure_time([&]() {
        auto sum = std::transform_reduce(
            std::execution::seq,
            data.begin(), data.end(),
            0LL,  // 初始值
            std::plus<>(),  // 归约操作
            [](int x) { return static_cast<long long>(x) * x; }  // 转换操作
        );
        volatile auto s = sum;
        (void)s;
    }, "transform_reduce seq");

    if constexpr (HAS_PARALLEL) {
        auto t3 = basic_parallel::measure_time([&]() {
            auto sum = std::transform_reduce(
                std::execution::par,
                data.begin(), data.end(),
                0LL,
                std::plus<>(),
                [](int x) { return static_cast<long long>(x) * x; }
            );
            volatile auto s = sum;
            (void)s;
        }, "transform_reduce par");

        if (t1 > 0) {
            std::cout << "  相对普通循环加速比: " << (t1 / t3) << "x\n";
        }
    }

    // 验证结果
    long long expected = static_cast<long long>(N) * (N + 1) * (2 * N + 1) / 6;
    long long actual = std::transform_reduce(
        std::execution::seq,
        data.begin(), data.end(),
        0LL, std::plus<>(),
        [](int x) { return static_cast<long long>(x) * x; }
    );
    std::cout << "  验证: 1^2 + 2^2 + ... + N^2 = " << actual
              << " (期望值: " << expected << ")\n";
}

// 3.3 内积（点积）计算
void demo_inner_product() {
    std::cout << "\n===== 3.3 向量内积（transform_reduce 应用） =====\n";

    const size_t N = 500'000;
    std::vector<double> a(N), b(N);
    std::iota(a.begin(), a.end(), 1.0);
    std::iota(b.begin(), b.end(), 2.0);

    double dot_product = std::transform_reduce(
        std::execution::seq,
        a.begin(), a.end(),
        b.begin(),  // 第二个范围
        0.0
    );

    std::cout << "向量维度: " << N << "\n";
    std::cout << "内积结果: " << dot_product << "\n";
    std::cout << "(transform_reduce 可以接受两个范围，计算对应元素乘积之和)\n";
}

} // namespace numeric_parallel


// ============================================================
// 第四部分：数据竞争与线程安全
// ============================================================

namespace thread_safety {

// 4.1 错误示例：并行 for_each 中的数据竞争
// 这是初学者最容易犯的错误

void demo_data_race() {
    std::cout << "\n===== 4.1 数据竞争（错误示范） =====\n";

    const size_t N = 100'000;
    std::vector<int> data(N, 1);

    // 错误！多个线程同时写 sum，数据竞争
    int sum = 0;
    // std::for_each(std::execution::par, data.begin(), data.end(),
    //     [&sum](int x) { sum += x; });  // 数据竞争！未定义行为

    // 正确方式1：使用 std::reduce
    int correct_sum = std::reduce(std::execution::seq, data.begin(), data.end(), 0);
    std::cout << "正确方式1 (reduce): sum = " << correct_sum << "\n";

    // 正确方式2：使用原子操作（性能较差）
    std::atomic<int> atomic_sum{0};
    if constexpr (HAS_PARALLEL) {
        std::for_each(std::execution::par, data.begin(), data.end(),
            [&atomic_sum](int x) { atomic_sum += x; });
        std::cout << "正确方式2 (atomic): sum = " << atomic_sum.load()
                  << " (但性能差，不如用 reduce)\n";
    } else {
        std::cout << "正确方式2 (atomic): 需要并行环境演示\n";
    }

    // 正确方式3：使用互斥锁（性能最差）
    std::mutex mtx;
    int mutex_sum = 0;
    // （不演示了，因为太慢）
    std::cout << "正确方式3 (mutex): 性能最差，不推荐\n";
}

// 4.2 并行算法中的异常处理
// 并行算法中如果有异常抛出，会调用 std::terminate（或由实现决定）
// 所以用户代码应尽量避免抛出异常

void demo_exception_safety() {
    std::cout << "\n===== 4.2 异常安全 =====\n";
    std::cout << "并行算法中的异常：\n";
    std::cout << "  - 标准要求：如果元素访问函数抛出异常，\n";
    std::cout << "    调用 std::terminate （终止程序）\n";
    std::cout << "  - 建议：确保并行代码不抛出异常\n";
    std::cout << "  - 可以用 noexcept 标记 lambda，编译器会优化\n";
    std::cout << "  - 如果需要错误处理，考虑用错误码或原子标志位\n";
}

// 4.3 迭代器要求
void demo_iterator_requirements() {
    std::cout << "\n===== 4.3 迭代器要求 =====\n";
    std::cout << "并行算法对迭代器的要求：\n";
    std::cout << "  - par 策略：至少是前向迭代器 (ForwardIterator)\n";
    std::cout << "  - par_unseq：必须是随机访问迭代器 (RandomAccessIterator)\n";
    std::cout << "  - 元素访问必须是独立的（不能有依赖关系）\n\n";
    std::cout << "不适合并行化的场景：\n";
    std::cout << "  - std::list（不是随机访问，且内存不连续）\n";
    std::cout << "  - 输入/输出迭代器（只能遍历一次）\n";
    std::cout << "  - 元素之间有依赖的计算\n";
}

} // namespace thread_safety


// ============================================================
// 第五部分：更多并行算法
// ============================================================

namespace more_algorithms {

// 5.1 并行查找
void demo_parallel_find() {
    std::cout << "\n===== 5.1 并行查找 find / find_if =====\n";

    const size_t N = 10'000'000;
    auto data = basic_parallel::generate_random_data(N);

    // 在数据中间位置放一个特殊值
    data[N / 2] = -999;

    auto t1 = basic_parallel::measure_time([&]() {
        auto it = std::find(std::execution::seq, data.begin(), data.end(), -999);
        volatile auto idx = it - data.begin();
        (void)idx;
    }, "find seq");

    if constexpr (HAS_PARALLEL) {
        auto t2 = basic_parallel::measure_time([&]() {
            auto it = std::find(std::execution::par, data.begin(), data.end(), -999);
            volatile auto idx = it - data.begin();
            (void)idx;
        }, "find par");

        if (t1 > 0) {
            std::cout << "  加速比: " << (t1 / t2) << "x\n";
        }
    } else {
        std::cout << "  (跳过 par 演示)\n";
    }
}

// 5.2 并行计数
void demo_parallel_count() {
    std::cout << "\n===== 5.2 并行计数 count / count_if =====\n";

    const size_t N = 5'000'000;
    auto data = basic_parallel::generate_random_data(N);

    // 统计大于 500000 的元素数量
    auto t1 = basic_parallel::measure_time([&]() {
        auto count = std::count_if(std::execution::seq, data.begin(), data.end(),
            [](int x) { return x > 500000; });
        volatile auto c = count;
        (void)c;
    }, "count_if seq");

    if constexpr (HAS_PARALLEL) {
        auto t2 = basic_parallel::measure_time([&]() {
            auto count = std::count_if(std::execution::par, data.begin(), data.end(),
                [](int x) { return x > 500000; });
            volatile auto c = count;
            (void)c;
        }, "count_if par");

        if (t1 > 0) {
            std::cout << "  加速比: " << (t1 / t2) << "x\n";
        }
    }

    auto count = std::count_if(std::execution::seq, data.begin(), data.end(),
        [](int x) { return x > 500000; });
    std::cout << "  大于 500000 的元素数: " << count
              << " (约 " << (count * 100.0 / N) << "%)\n";
}

// 5.3 并行变换
void demo_parallel_transform() {
    std::cout << "\n===== 5.3 并行 transform =====\n";

    const size_t N = 2'000'000;
    std::vector<int> input(N);
    std::iota(input.begin(), input.end(), 1);
    std::vector<int> output(N);

    auto t1 = basic_parallel::measure_time([&]() {
        std::transform(std::execution::seq,
            input.begin(), input.end(), output.begin(),
            [](int x) { return x * x + 2 * x + 1; }  // (x+1)^2
        );
    }, "transform seq");

    if constexpr (HAS_PARALLEL) {
        std::vector<int> output2(N);
        auto t2 = basic_parallel::measure_time([&]() {
            std::transform(std::execution::par,
                input.begin(), input.end(), output2.begin(),
                [](int x) { return x * x + 2 * x + 1; }
            );
        }, "transform par");

        if (t1 > 0) {
            std::cout << "  加速比: " << (t1 / t2) << "x\n";
        }
    }

    std::cout << "  transform 适合 CPU 密集型的元素独立计算\n";
}

} // namespace more_algorithms


// ============================================================
// 第六部分：性能考量与最佳实践
// ============================================================

namespace best_practices {

// 6.1 何时使用并行算法
void demo_when_to_use() {
    std::cout << "\n===== 6.1 何时使用并行算法 =====\n";

    std::cout << "适合并行化的场景：\n";
    std::cout << "  - 数据量大（至少几万以上元素）\n";
    std::cout << "  - 每个元素的计算独立（无依赖）\n";
    std::cout << "  - CPU 密集型计算（不是 I/O 密集型）\n";
    std::cout << "  - 计算开销足够大（超过并行调度开销）\n\n";

    std::cout << "不适合并行化的场景：\n";
    std::cout << "  - 数据量小（并行调度开销 > 计算开销）\n";
    std::cout << "  - 元素之间有依赖关系\n";
    std::cout << "  - 需要频繁加锁或同步\n";
    std::cout << "  - 内存带宽受限（内存密集型）\n";
}

// 6.2 常见的性能陷阱
void demo_performance_pitfalls() {
    std::cout << "\n===== 6.2 性能陷阱 =====\n";

    std::cout << "陷阱1：小数据量用并行\n";
    std::cout << "  - 线程创建/同步有开销\n";
    std::cout << "  - 经验法则：元素计算时间 > 1 微秒才考虑并行\n\n";

    std::cout << "陷阱2：内存带宽瓶颈\n";
    std::cout << "  - 简单运算（如加法）往往受限于内存带宽\n";
    std::cout << "  - 增加 CPU 核心数不会线性提升性能\n";
    std::cout << "  - 计算越复杂，并行收益越大\n\n";

    std::cout << "陷阱3：假共享 (False Sharing)\n";
    std::cout << "  - 不同线程访问同一缓存行的数据\n";
    std::cout << "  - 导致缓存行频繁失效，性能急剧下降\n";
    std::cout << "  - 解决：按块处理数据，或使用对齐的局部变量\n\n";

    std::cout << "陷阱4：过度并行\n";
    std::cout << "  - 嵌套并行（并行算法中再调用并行算法）\n";
    std::cout << "  - 可能导致线程爆炸和调度开销\n";
    std::cout << "  - 建议：只在最外层使用并行\n";
}

// 6.3 最佳实践清单
void demo_checklist() {
    std::cout << "\n===== 6.3 最佳实践清单 =====\n";

    std::cout << "1. 先用串行实现，测量性能瓶颈\n";
    std::cout << "2. 确认计算是可并行的（元素独立）\n";
    std::cout << "3. 选择合适的执行策略（par vs par_unseq）\n";
    std::cout << "4. 使用合适的算法（reduce 比手写 for_each+原子更好）\n";
    std::cout << "5. 测量并行前后的性能对比\n";
    std::cout << "6. 确保没有数据竞争\n";
    std::cout << "7. 考虑内存布局对缓存的影响\n";
    std::cout << "8. 避免嵌套并行\n";
}

} // namespace best_practices


// ============================================================
// 第七部分：综合示例 - 图像处理模拟
// ============================================================

namespace image_processing {

// 模拟一个简单的图像处理流程
// 用一维数组表示图像像素（灰度图）

struct Image {
    int width;
    int height;
    std::vector<float> pixels;  // 像素值 0.0 - 1.0

    Image(int w, int h) : width(w), height(h), pixels(w * h, 0.0f) {}
};

// 生成测试图像
Image generate_test_image(int width, int height) {
    Image img(width, height);
    std::mt19937 gen(123);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (auto& p : img.pixels) p = dist(gen);
    return img;
}

// 亮度调整（点操作，完全可并行）
void adjust_brightness(Image& img, float factor) {
    std::for_each(std::execution::seq, img.pixels.begin(), img.pixels.end(),
        [factor](float& p) {
            p = std::clamp(p * factor, 0.0f, 1.0f);
        }
    );
}

// 计算图像平均亮度
float average_brightness(const Image& img) {
    float sum = std::reduce(std::execution::seq, img.pixels.begin(), img.pixels.end(), 0.0f);
    return sum / img.pixels.size();
}

// 直方图统计
std::vector<int> compute_histogram(const Image& img, int bins = 256) {
    std::vector<int> histogram(bins, 0);
    // 注意：histogram 的更新有数据竞争风险
    // 正确做法：用 transform_reduce 或 每个线程独立的直方图再合并
    // 这里用串行版本演示（并行版本需要更复杂的实现）
    for (float p : img.pixels) {
        int bin = static_cast<int>(p * (bins - 1));
        histogram[bin]++;
    }
    return histogram;
}

void demo_image_processing() {
    std::cout << "\n===== 7. 综合示例：图像处理 =====\n";

    const int width = 2000;
    const int height = 2000;
    std::cout << "图像尺寸: " << width << " x " << height
              << " = " << (width * height) << " 像素\n\n";

    auto img = generate_test_image(width, height);

    // 原始平均亮度
    float avg1 = average_brightness(img);
    std::cout << "原始平均亮度: " << avg1 << "\n";

    // 亮度调整
    auto t1 = basic_parallel::measure_time([&]() {
        adjust_brightness(img, 1.5f);
    }, "亮度调整 (seq)");

    float avg2 = average_brightness(img);
    std::cout << "调整后平均亮度: " << avg2 << "\n";

    // 直方图
    auto hist = compute_histogram(img);
    int max_bin = std::distance(hist.begin(), std::max_element(hist.begin(), hist.end()));
    std::cout << "直方图峰值位置: " << max_bin << "/255\n";

    if constexpr (HAS_PARALLEL) {
        auto img2 = generate_test_image(width, height);
        auto t2 = basic_parallel::measure_time([&]() {
            std::for_each(std::execution::par, img2.pixels.begin(), img2.pixels.end(),
                [](float& p) { p = std::clamp(p * 1.5f, 0.0f, 1.0f); }
            );
        }, "亮度调整 (par)");

        if (t1 > 0) {
            std::cout << "  并行加速比: " << (t1 / t2) << "x\n";
        }
    }

    std::cout << "\n图像处理中的常见可并行操作:\n";
    std::cout << "  - 点操作（亮度、对比度、颜色转换）: 完美并行\n";
    std::cout << "  - 卷积/滤波: 需要邻域访问，较复杂\n";
    std::cout << "  - 直方图统计: 需要归约操作\n";
    std::cout << "  - 图像变换（缩放、旋转）: 可并行但有读写依赖\n";
}

} // namespace image_processing


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 并行算法（Parallel Algorithms）深度解析\n";
    std::cout << "============================================\n";

    execution_policy::demo_policy_concepts();

    basic_parallel::demo_parallel_sort();
    basic_parallel::demo_parallel_for_each();

    numeric_parallel::demo_reduce();
    numeric_parallel::demo_transform_reduce();
    numeric_parallel::demo_inner_product();

    thread_safety::demo_data_race();
    thread_safety::demo_exception_safety();
    thread_safety::demo_iterator_requirements();

    more_algorithms::demo_parallel_find();
    more_algorithms::demo_parallel_count();
    more_algorithms::demo_parallel_transform();

    best_practices::demo_when_to_use();
    best_practices::demo_performance_pitfalls();
    best_practices::demo_checklist();

    image_processing::demo_image_processing();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
