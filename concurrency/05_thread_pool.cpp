// ============================================================
// 05_thread_pool.cpp - 线程池
// 编译命令: g++ -std=c++17 -pthread 05_thread_pool.cpp -o 05_thread_pool
// ============================================================

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <chrono>
#include <atomic>
#include <numeric>
#include <algorithm>

using namespace std;

// ============================================================
// 一、为什么需要线程池？
// ============================================================
// 老师傅讲故事：
// 想象你开了个饭店，客人来一个，你就去人才市场招一个服务员，
// 客人走了，你就把服务员开了。这样搞下来，你饭店不用开了，
// 光在招人开人的路上跑了——招人要时间，开人也要成本啊！
//
// 线程也是一样的道理：
//   - 创建一个线程需要分配栈空间、注册到操作系统...开销不小
//   - 销毁线程也要回收资源
//   - 如果任务很短小，创建销毁的开销可能比任务本身还大
//
// 线程池就是"提前雇好一批服务员"：
//   - 程序启动时创建一堆线程，让它们待命
//   - 有任务来了，扔给空闲的线程去做
//   - 做完了，线程回到池中继续待命
//   - 程序退出时，统一销毁所有线程
//
// 这样就避免了频繁创建销毁线程的开销。
// ============================================================

// ============================================================
// 二、线程池的基本组成
// ============================================================
// 老师傅画图纸：
// 一个最简单的线程池，有这几样东西：
//
//   1. 任务队列（Task Queue）
//      - 就像饭店里的"订单板"
//      - 主线程把任务写上去，工作线程从上面取任务
//
//   2. 工作线程（Worker Threads）
//      - 就是那些"服务员"
//      - 不停地从任务队列取任务、执行任务
//      - 没任务的时候就睡觉（等待）
//
//   3. 同步机制
//      - mutex 保护任务队列（防止多个线程同时取任务乱套）
//      - condition_variable 通知工作线程"来活了"
//
//   4. 停止标志
//      - 告诉线程们"下班了，别等了"
// ============================================================

// ============================================================
// 三、实现一个简单的线程池
// ============================================================

class ThreadPool {
private:
    vector<thread> workers;          // 工作线程们
    queue<function<void()>> tasks;   // 任务队列

    mutex queue_mutex;               // 保护任务队列的互斥锁
    condition_variable condition;    // 条件变量，通知线程"有活了"
    bool stop{false};                // 停止标志

public:
    // ---- 构造函数：创建指定数量的工作线程 ----
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this, i]() {
                // 每个工作线程的主循环
                while (true) {
                    function<void()> task;

                    {
                        // 拿锁，等任务
                        unique_lock<mutex> lock(this->queue_mutex);
                        // 条件变量等待：
                        // - stop 为 true 时，不等了，下班
                        // - 任务队列非空时，有活干了
                        this->condition.wait(lock, [this]() {
                            return this->stop || !this->tasks.empty();
                        });

                        // 如果停止了而且没任务了，就退出循环
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }

                        // 从队列里取一个任务
                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // 执行任务（注意：在锁外面执行！）
                    try {
                        task();
                    } catch (const exception& e) {
                        cerr << "线程" << i << "执行任务时异常: " << e.what() << endl;
                    }
                }
            });
        }
    }

    // ---- 提交任务：返回 future，方便获取结果 ----
    // 用模板，支持任意函数和参数
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> future<invoke_result_t<F, Args...>> {
        using ReturnType = invoke_result_t<F, Args...>;

        // 把任务包装成 packaged_task，这样可以拿到 future
        auto task = make_shared<packaged_task<ReturnType()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );

        future<ReturnType> result = task->get_future();

        {
            unique_lock<mutex> lock(queue_mutex);

            // 如果已经停止了，就不允许再加任务了
            if (stop) {
                throw runtime_error("线程池已停止，不能再提交任务");
            }

            // 把任务塞进队列
            tasks.emplace([task]() {
                (*task)();
            });
        }

        // 通知一个工作线程：来活了！
        condition.notify_one();

        return result;
    }

    // ---- 析构函数：停止所有线程 ----
    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;  // 设置停止标志
        }

        // 通知所有线程：下班了！
        condition.notify_all();

        // 等所有线程干完手头的活再走
        for (auto& worker : workers) {
            worker.join();
        }
    }

    // 禁止拷贝和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};

// ============================================================
// 演示1：基本用法 - 提交任务、获取结果
// ============================================================
void demo_basic_usage() {
    cout << "\n=== 演示1：线程池基本用法 ===" << endl;

    // 创建一个有4个线程的线程池
    ThreadPool pool(4);
    cout << "创建了一个4线程的线程池" << endl;

    // 提交几个简单的任务
    vector<future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i]() {
                cout << "任务" << i << "开始执行（线程ID: "
                     << this_thread::get_id() << "）" << endl;
                this_thread::sleep_for(chrono::milliseconds(100));  // 模拟干活
                return i * i;
            })
        );
    }

    // 获取结果
    cout << "\n获取结果：" << endl;
    for (size_t i = 0; i < results.size(); ++i) {
        cout << "  任务" << i << "的结果: " << results[i].get() << endl;
    }

    cout << "你看，4个线程处理8个任务，线程复用了，不用每次创建新线程。" << endl;
}

// ============================================================
// 演示2：并行求和（分治思想）
// ============================================================
// 老师傅讲思路：
// 要算一个很大的数组的和，一个线程算太慢了，
// 可以把数组切成好几块，每块扔给线程池里的一个线程去算，
// 最后把各块的结果加起来。这就是"分治"思想。
//
// 就像搬砖，一个人搬1000块砖累死人，
// 分给4个人，每人搬250块，搬完汇总就行。
// ============================================================

// 计算数组某一段的和
long long partial_sum(const vector<int>& arr, size_t start, size_t end) {
    long long sum = 0;
    for (size_t i = start; i < end; ++i) {
        sum += arr[i];
    }
    return sum;
}

void demo_parallel_sum() {
    cout << "\n=== 演示2：并行求和 ===" << endl;

    // 构造一个大数组
    const size_t N = 10000000;
    vector<int> arr(N);
    for (size_t i = 0; i < N; ++i) {
        arr[i] = i % 100;  // 简单点，不用随机
    }

    // ---- 单线程求和（作为基准） ----
    auto start = chrono::steady_clock::now();
    long long single_sum = partial_sum(arr, 0, N);
    auto end = chrono::steady_clock::now();
    auto single_time = chrono::duration<double, milli>(end - start).count();
    cout << "单线程求和: " << single_sum << "，耗时: " << single_time << "ms" << endl;

    // ---- 线程池并行求和 ----
    const int num_threads = 4;
    ThreadPool pool(num_threads);

    start = chrono::steady_clock::now();

    // 把数组分成 num_threads 块
    vector<future<long long>> futures;
    size_t block_size = N / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        size_t start_idx = i * block_size;
        size_t end_idx = (i == num_threads - 1) ? N : (i + 1) * block_size;
        futures.emplace_back(
            pool.enqueue(partial_sum, cref(arr), start_idx, end_idx)
        );
    }

    // 汇总结果
    long long parallel_sum = 0;
    for (auto& f : futures) {
        parallel_sum += f.get();
    }

    end = chrono::steady_clock::now();
    auto parallel_time = chrono::duration<double, milli>(end - start).count();
    cout << "并行求和:   " << parallel_sum << "，耗时: " << parallel_time << "ms" << endl;

    // 验证结果
    cout << "结果" << (single_sum == parallel_sum ? "一致" : "不一致！") << endl;
    cout << "加速比: " << (single_time / parallel_time) << " 倍" << endl;
    cout << "（注意：如果你的CPU核心数少，加速比可能不明显，甚至更慢，因为线程调度也有开销）" << endl;
}

// ============================================================
// 演示3：异步任务处理（模拟Web服务器）
// ============================================================
// 老师傅讲场景：
// 想象你写了个Web服务器，每个请求就是一个任务。
// 如果来一个请求就开一个线程，请求多了系统就崩了。
// 用线程池就可以控制并发数，保证系统稳定。
// ============================================================

void demo_async_tasks() {
    cout << "\n=== 演示3：异步任务处理（模拟Web请求） ===" << endl;

    ThreadPool pool(3);  // 3个工作线程，模拟服务器处理能力

    atomic<int> request_id{0};

    // 模拟来了10个请求
    vector<future<string>> responses;
    for (int i = 0; i < 10; ++i) {
        int id = request_id.fetch_add(1);
        responses.emplace_back(
            pool.enqueue([id]() -> string {
                cout << "[请求" << id << "] 开始处理..." << endl;
                // 模拟处理时间：有的快有的慢
                this_thread::sleep_for(chrono::milliseconds(50 + (id % 3) * 50));
                cout << "[请求" << id << "] 处理完成" << endl;
                return "响应" + to_string(id) + ": OK";
            })
        );
    }

    // 收集响应
    cout << "\n收集响应：" << endl;
    for (auto& f : responses) {
        cout << "  " << f.get() << endl;
    }

    cout << "你看，虽然来了10个请求，但同时处理的只有3个，" << endl;
    cout << "系统不会被冲垮，这就是线程池的"限流"作用。" << endl;
}

// ============================================================
// 演示4：线程数调优 - CPU密集型 vs IO密集型
// ============================================================
// 老师傅讲经验：
// 线程池设多少个线程合适？这是个好问题。
//
// 分两种情况：
//
// 1. CPU密集型任务（计算为主）
//    - 比如：数学计算、数据压缩、图像处理
//    - 线程数 = CPU核心数 或者 CPU核心数+1
//    - 太多了反而慢，因为线程切换有开销
//
// 2. IO密集型任务（等待为主）
//    - 比如：网络请求、文件读写、数据库查询
//    - 线程数可以设大一点，比如 2*CPU核心数 或者更多
//    - 因为线程大部分时间在等IO，多开几个可以让CPU别闲着
//
// 简单公式（Little's Law的简化版）：
//   最佳线程数 = CPU核心数 * (1 + 等待时间/计算时间)
//
// 老师傅的话：
//   这都是经验值，实际项目中要靠压测来调。
//   没有银弹，压测出真知。
// ============================================================

void demo_thread_count_tuning() {
    cout << "\n=== 演示4：线程数调优 - CPU密集型 vs IO密集型 ===" << endl;

    const int num_tasks = 20;
    unsigned int cpu_cores = thread::hardware_concurrency();
    cout << "你的CPU核心数: " << cpu_cores << endl;

    // ---- CPU密集型任务 ----
    cout << "\n--- CPU密集型任务（计算斐波那契） ---" << endl;

    auto cpu_task = [](int n) -> long long {
        // 故意写个低效的递归斐波那契，模拟CPU计算
        function<long long(int)> fib = [&fib](int n) -> long long {
            if (n <= 1) return n;
            return fib(n-1) + fib(n-2);
        };
        return fib(n);
    };

    // 用不同线程数测试
    for (int num_threads : {1, 2, 4}) {
        ThreadPool pool(num_threads);

        auto start = chrono::steady_clock::now();
        vector<future<long long>> results;
        for (int i = 0; i < num_tasks; ++i) {
            results.emplace_back(pool.enqueue(cpu_task, 25));
        }
        for (auto& f : results) f.get();
        auto end = chrono::steady_clock::now();

        auto time = chrono::duration<double, milli>(end - start).count();
        cout << "  " << num_threads << "线程: " << time << "ms" << endl;
    }

    cout << "\nCPU密集型：线程数接近核心数时性能最好，再多反而下降。" << endl;

    // ---- IO密集型任务 ----
    cout << "\n--- IO密集型任务（模拟等待） ---" << endl;

    auto io_task = []() {
        // 模拟IO等待：大部分时间在睡觉
        this_thread::sleep_for(chrono::milliseconds(100));
        return 42;
    };

    for (int num_threads : {2, 5, 10, 20}) {
        ThreadPool pool(num_threads);

        auto start = chrono::steady_clock::now();
        vector<future<int>> results;
        for (int i = 0; i < num_tasks; ++i) {
            results.emplace_back(pool.enqueue(io_task));
        }
        for (auto& f : results) f.get();
        auto end = chrono::steady_clock::now();

        auto time = chrono::duration<double, milli>(end - start).count();
        cout << "  " << num_threads << "线程: " << time << "ms" << endl;
    }

    cout << "\nIO密集型：线程越多越快（直到任务数上限），" << endl;
    cout << "因为线程大部分时间在等，多开几个能让CPU别闲着。" << endl;
}

// ============================================================
// 演示5：线程池的扩展方向
// ============================================================
// 老师傅讲进阶：
// 咱们这个线程池是最简版的，工业级的线程池功能多着呢：
//
// 1. 任务优先级
//    - 有的任务紧急，有的不紧急
//    - 用 priority_queue 代替普通 queue
//    - 比如：支付任务优先级 > 日志任务
//
// 2. 动态调整线程数
//    - 任务多了自动加线程，任务少了自动减
//    - 核心线程数 + 最大线程数
//    - 就像饭店饭点加临时工，非饭点让临时工下班
//
// 3. 任务拒绝策略
//    - 任务队列满了怎么办？
//    - 常见策略：直接拒绝、调用者自己跑、丢最老的任务、阻塞等待
//
// 4. 线程池监控
//    - 当前活跃线程数、队列里任务数、已完成任务数...
//
// 5. 优雅关闭
//    - shutdown：等现有任务做完，不接新任务
//    - shutdownNow：直接停，队列里的任务不管了
//
// 有兴趣可以看看 Java 的 ThreadPoolExecutor 源码，
// 设计得非常经典，C++很多线程池都是照着那个思路实现的。
// ============================================================

// ---- 带优先级的任务（演示思路，不完整实现） ----
struct PriorityTask {
    int priority;    // 优先级，数字越大越优先
    function<void()> task;

    // 为 priority_queue 提供比较函数（注意是"小于"，返回true的排后面）
    bool operator<(const PriorityTask& other) const {
        return priority < other.priority;  // 优先级高的在前
    }
};

void demo_pool_extensions() {
    cout << "\n=== 演示5：线程池的扩展方向（概念演示） ===" << endl;

    cout << "1. 任务优先级：用 priority_queue 代替普通 queue" << endl;
    cout << "   高优先级任务会被优先执行" << endl;
    cout << "" << endl;
    cout << "2. 动态调整线程数：" << endl;
    cout << "   - 核心线程：一直活着的线程" << endl;
    cout << "   - 非核心线程：任务多时临时加的，闲了就销毁" << endl;
    cout << "" << endl;
    cout << "3. 任务拒绝策略：" << endl;
    cout << "   - AbortPolicy: 直接抛异常" << endl;
    cout << "   - CallerRunsPolicy: 提交任务的线程自己跑" << endl;
    cout << "   - DiscardPolicy: 悄悄丢掉任务" << endl;
    cout << "   - DiscardOldestPolicy: 丢掉最老的任务" << endl;
    cout << "" << endl;
    cout << "4. 监控指标：活跃线程数、队列长度、完成任务数..." << endl;
    cout << "" << endl;
    cout << "（这些都是工业级线程池的特性，咱们的简单版没实现，了解一下就行）" << endl;
}

// ============================================================
// 补充：一种更现代的写法 - 用 packaged_task 和 可变参数模板
// ============================================================
// 老师傅说：
// 咱们上面的 ThreadPool::enqueue 已经用了模板和 packaged_task，
// 可以接受任意函数和参数，返回 future，已经挺好用的了。
// 这里再补充几个使用场景，让你感受一下线程池的方便。
// ============================================================

void demo_various_tasks() {
    cout << "\n=== 演示6：线程池处理各种类型的任务 ===" << endl;

    ThreadPool pool(2);

    // 任务1：普通函数
    auto f1 = pool.enqueue([]() {
        cout << "  任务1：无返回值的任务" << endl;
    });

    // 任务2：带返回值的
    auto f2 = pool.enqueue([](int a, int b) {
        return a + b;
    }, 10, 20);

    // 任务3：带引用参数（要用 ref 包装）
    int counter = 0;
    mutex mtx;
    auto f3 = pool.enqueue([&counter, &mtx]() {
        lock_guard<mutex> lock(mtx);
        counter += 100;
    });

    // 任务4：异常处理
    auto f4 = pool.enqueue([]() -> int {
        throw runtime_error("故意抛个异常玩玩");
    });

    // 等待所有任务完成
    f1.get();
    cout << "  任务2结果: " << f2.get() << endl;
    f3.get();
    cout << "  任务3执行后 counter = " << counter << endl;

    try {
        f4.get();
    } catch (const exception& e) {
        cout << "  任务4捕获到异常: " << e.what() << endl;
    }

    cout << "线程池可以处理各种类型的任务，灵活得很。" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "============================================================" << endl;
    cout << "  并发编程进阶05：线程池" << endl;
    cout << "============================================================" << endl;

    demo_basic_usage();        // 基本用法
    demo_parallel_sum();       // 并行求和
    demo_async_tasks();        // 异步任务处理
    demo_thread_count_tuning(); // 线程数调优
    demo_pool_extensions();    // 扩展方向
    demo_various_tasks();      // 各种任务类型

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "徒弟啊，线程池这东西，是项目里的标配，比你想的更常用。" << endl;
    cout << "" << endl;
    cout << "1. 为什么用线程池？" << endl;
    cout << "   核心就是俩字：复用。避免频繁创建销毁线程的开销。" << endl;
    cout << "   就像饭店不能来一个客人招一个服务员，走一个开一个。" << endl;
    cout << "" << endl;
    cout << "2. 线程池的核心三件套：" << endl;
    cout << "   任务队列 + 工作线程 + condition_variable 通知" << endl;
    cout << "   记住这三样，你就能写一个最基本的线程池了。" << endl;
    cout << "" << endl;
    cout << "3. 线程数设多少？" << endl;
    cout << "   CPU密集型：核心数左右" << endl;
    cout << "   IO密集型：可以多一些，看等待时间比例" << endl;
    cout << "   但记住，这都是经验值，最终还是要靠压测调优。" << endl;
    cout << "" << endl;
    cout << "4. 实际项目中，尽量用成熟的库。" << endl;
    cout << "   比如 Intel TBB 的 task_arena，或者自己封装一个。" << endl;
    cout << "   咱们写的这个是学习用的，生产环境还要考虑很多边界情况。" << endl;
    cout << "" << endl;
    cout << "5. C++20 有了 std::jthread，还有 std::execution 的并行算法，" << endl;
    cout << "   但线程池的思想是通用的，底层原理万变不离其宗。" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习" << endl;
    cout << "============================================================" << endl;
    cout << "1. 给咱们的线程池加一个 shutdown() 方法：" << endl;
    cout << "   调用后不再接受新任务，但已提交的任务继续执行完。" << endl;
    cout << "" << endl;
    cout << "2. 实现一个带优先级的线程池：" << endl;
    cout << "   用 priority_queue 存储任务，高优先级的任务先执行。" << endl;
    cout << "   （提示：任务需要包装成带优先级的结构体）" << endl;
    cout << "" << endl;
    cout << "3. 用线程池实现并行排序：" << endl;
    cout << "   把数组分成几块，每块并行排序，最后合并。" << endl;
    cout << "   （提示：可以用 std::inplace_merge 合并）" << endl;
    cout << "" << endl;
    cout << "4. 思考：如果任务队列里的任务特别多，内存会不会爆？" << endl;
    cout << "   怎么实现一个有界队列（有最大容量限制）？" << endl;
    cout << "   队列满了的时候，提交任务的行为应该是怎样的？" << endl;
    cout << "" << endl;
    cout << "5. 查一下什么是"工作窃取"（Work Stealing）：" << endl;
    cout << "   为什么有的线程池每个线程有自己的队列？" << endl;
    cout << "   这和只有一个全局队列的方案比有什么优缺点？" << endl;

    return 0;
}
