// ============================================================
// 06_async_future.cpp - 异步编程：future/promise
// 编译命令: g++ -std=c++17 -pthread 06_async_future.cpp -o 06_async_future
// ============================================================

#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <exception>
#include <functional>
#include <string>
#include <mutex>
#include <numeric>
#include <algorithm>

using namespace std;

// ============================================================
// 一、什么是异步编程？
// ============================================================
// 老师傅讲故事：
// 你去面馆吃面，跟老板说"来碗牛肉面"。
// 两种等法：
//
// 1. 同步：你站在窗口前死等，面没做好你啥也不干，就盯着锅。
//    （调用函数，等它返回，期间啥也不干）
//
// 2. 异步：老板给你个取餐号，你找个座儿玩手机，
//    等面做好了叫号，你再去取。
//    （启动任务，拿到一个"凭证"，先去干别的，
//     需要结果的时候再用凭证去取）
//
// C++里的 future 就是那个"取餐号"，
// 你拿着 future，可以选择：
//   - 等（wait）
//   - 等一会儿看看好了没（wait_for）
//   - 直接取餐（get）—— 没好就等
// ============================================================

// ============================================================
// 二、std::async - 最简单的异步方式
// ============================================================
// 老师傅说：
// std::async 是C++标准库给我们的"一键异步"工具，
// 你给它一个函数，它就帮你异步执行，返回一个 future。
//
// 两种启动策略：
//   1. launch::async - 马上开个新线程执行（"老板，赶紧做！"）
//   2. launch::deferred - 延迟执行，等你调用 get() 时才执行
//      （"面先别急着做，等我来了再下锅"）
//
// 如果你不指定，默认是 launch::async | launch::deferred，
// 由实现选择其中一种；标准没有规定选择依据，
// 常见实现（libstdc++/MSVC STL）通常直接选 async。
// ============================================================

// 一个耗时的计算任务
int slow_calculation(int x) {
    cout << "  [慢计算] 开始计算 " << x << " 的平方..." << endl;
    this_thread::sleep_for(chrono::milliseconds(500));  // 模拟耗时
    int result = x * x;
    cout << "  [慢计算] 完成，结果 = " << result << endl;
    return result;
}

void demo_async_basic() {
    cout << "\n=== 演示1：std::async 基本用法 ===" << endl;

    cout << "主线程：启动异步任务..." << endl;

    // 用 async 启动异步任务
    future<int> f = async(launch::async, slow_calculation, 42);

    cout << "主线程：异步任务已启动，我先干点别的..." << endl;

    // 干点别的事
    for (int i = 0; i < 3; ++i) {
        cout << "主线程：忙别的事中..." << i << endl;
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    cout << "主线程：好了，我要结果了..." << endl;

    // get() 会阻塞等待结果就绪
    int result = f.get();
    cout << "主线程：拿到结果了，" << 42 << "的平方是 " << result << endl;
}

// ---- launch::async vs launch::deferred 对比 ----
void demo_async_launch_policy() {
    cout << "\n=== 演示2：launch::async vs launch::deferred ===" << endl;

    auto task = [](int id) {
        cout << "  任务" << id << "在执行（线程ID: "
             << this_thread::get_id() << "）" << endl;
        return id * 10;
    };

    cout << "主线程ID: " << this_thread::get_id() << endl;
    cout << endl;

    // async：立即在新线程执行
    cout << "--- launch::async ---" << endl;
    cout << "调用 async 前..." << endl;
    future<int> f1 = async(launch::async, task, 1);
    cout << "调用 async 后..." << endl;
    cout << "结果: " << f1.get() << endl;

    cout << endl;

    // deferred：延迟到 get() 时才执行，而且在当前线程执行
    cout << "--- launch::deferred ---" << endl;
    cout << "调用 async 前..." << endl;
    future<int> f2 = async(launch::deferred, task, 2);
    cout << "调用 async 后（任务还没执行呢）..." << endl;
    this_thread::sleep_for(chrono::milliseconds(200));
    cout << "准备调用 get()..." << endl;
    cout << "结果: " << f2.get() << endl;

    cout << "\n看到没？deferred 的任务在 get() 之前根本没跑，" << endl;
    cout << "而且是在调用 get() 的线程里跑的（线程ID一样）。" << endl;
}

// ============================================================
// 三、std::future 的各种等待方式
// ============================================================
// 老师傅讲解：
// future 不只有 get() 一种用法，还有几种等待方式：
//
//   wait()         - 一直等，等到天荒地老
//   wait_for(时间) - 等一段时间，超时了就不等了
//   wait_until(时间点) - 等到某个时间点
//
// wait_for/wait_until 返回三种状态：
//   future_status::ready   - 结果好了
//   future_status::timeout - 等超时了，还没好
//   future_status::deferred - 任务是延迟执行的，还没开始呢
//
// 比喻：
//   wait() 就是站在窗口前死等
//   wait_for(3秒) 就是看3秒手机，没好就先去干别的
// ============================================================

void demo_future_wait() {
    cout << "\n=== 演示3：future 的等待方式 ===" << endl;

    // 一个耗时2秒的任务
    future<int> f = async(launch::async, []() {
        this_thread::sleep_for(chrono::seconds(2));
        return 100;
    });

    cout << "主线程：开始轮询结果..." << endl;

    // 轮询方式：每隔一段时间看看好了没
    int attempts = 0;
    while (true) {
        // 等200毫秒看看
        future_status status = f.wait_for(chrono::milliseconds(200));
        attempts++;

        switch (status) {
            case future_status::ready:
                cout << "第" << attempts << "次检查：结果好了！" << endl;
                cout << "结果: " << f.get() << endl;
                return;
            case future_status::timeout:
                cout << "第" << attempts << "次检查：还没好，继续等..." << endl;
                break;
            case future_status::deferred:
                // 延迟任务不会自己开始；wait_for 会一直返回 deferred。
                // 必须调用 get()（或 wait()）才会真正执行，否则会死循环。
                cout << "任务是延迟执行的，现在调用 get() 触发执行" << endl;
                cout << "结果: " << f.get() << endl;
                return;
        }
    }
}

// ============================================================
// 四、std::promise - 手动设置结果
// ============================================================
// 老师傅讲原理：
// async 很方便，但有些场景不够灵活。
// 比如你想在一个已经存在的线程里设置结果，
// 或者结果要经过好几道手续才能出来。
//
// promise 就是"手动版"的异步：
//   - 你创建一个 promise，从它那拿到一个 future
//   - 你（或者别的线程）在某个时候调用 promise.set_value() 设置结果
//   - 持有 future 的人就能拿到结果了
//
// 比喻：
//   async 就像你去快餐店点餐，收银员直接给你取餐号，
//   厨房自动给你做，做好了叫号。
//
//   promise 就像你去私房菜馆，老板给你个号（future），
//   但什么时候上菜、谁来上菜，全看老板安排（promise）。
//   老板可以在任何时候、让任何人把菜端出来（set_value）。
// ============================================================

void demo_promise() {
    cout << "\n=== 演示4：std::promise 手动设置结果 ===" << endl;

    promise<int> p;          // 承诺：我会给你一个int
    future<int> f = p.get_future();  // 凭证：将来从这取结果

    // 启动一个线程，在某个时刻设置结果
    thread worker([p = move(p)]() mutable {
        // 注意：promise 不能拷贝，只能移动
        cout << "  工作线程：正在努力计算..." << endl;
        this_thread::sleep_for(chrono::milliseconds(500));

        int result = 666;
        cout << "  工作线程：算完了，设置结果 = " << result << endl;
        p.set_value(result);  // 设置结果！
    });

    cout << "主线程：等待结果..." << endl;
    cout << "主线程：拿到结果 = " << f.get() << endl;

    worker.join();
    cout << "promise 让你可以完全控制什么时候、在哪里设置结果，" << endl;
    cout << "比 async 更灵活，但也更手动。" << endl;
}

// ---- promise 还能设置异常 ----
void demo_promise_exception() {
    cout << "\n=== 演示5：promise 设置异常 ===" << endl;

    promise<int> p;
    future<int> f = p.get_future();

    thread worker([p = move(p)]() mutable {
        try {
            // 模拟出错
            throw runtime_error("计算出错啦！");
        } catch (...) {
            // 捕获异常，设置到 promise 里
            p.set_exception(current_exception());
        }
    });

    try {
        int result = f.get();
        cout << "结果: " << result << endl;
    } catch (const exception& e) {
        cout << "捕获到异常: " << e.what() << endl;
    }

    worker.join();
    cout << "promise 不仅能传结果，还能传异常，" << endl;
    cout << "这样工作线程里的异常就能被主线程捕获到了。" << endl;
}

// ============================================================
// 五、std::packaged_task - 把任务打包
// ============================================================
// 老师傅讲三者关系：
// 现在我们有三个东西了，容易混，咱理一理：
//
//   1. std::async
//      - 最方便，直接传函数，返回 future
//      - 标准不规定内部怎么实现：可能是新线程，也可能是线程池
//
//   2. std::packaged_task
//      - 把一个函数"打包"成一个任务，这个任务执行完，
//        它绑定的 future 就能拿到结果
//      - 比 async 灵活：你可以决定什么时候、在哪个线程执行
//
//   3. std::promise
//      - 最底层，最灵活
//      - 你手动控制什么时候 set_value
//      - 不绑定具体函数，你想怎么设置结果都行
//
// 比喻：
//   async      = 方便面，撕开泡开水就行，最省事
//   packaged_task = 半成品菜，你自己下锅炒一下
//   promise    = 原材料，你自己切、自己炒、自己调味
// ============================================================

int calculate_fib(int n) {
    if (n <= 1) return n;
    return calculate_fib(n-1) + calculate_fib(n-2);
}

void demo_packaged_task() {
    cout << "\n=== 演示6：std::packaged_task 打包任务 ===" << endl;

    // 把函数打包成 packaged_task
    packaged_task<int(int)> task(calculate_fib);

    // 从 task 拿到 future
    future<int> f = task.get_future();

    // packaged_task 本身是个可调用对象，可以在任何线程执行
    // 比如我们在一个新线程里执行
    thread t(move(task), 20);  // packaged_task 不能拷贝，只能移动

    cout << "主线程：等待斐波那契结果..." << endl;
    cout << "fib(20) = " << f.get() << endl;

    t.join();

    cout << "\npackaged_task 把函数和 future 绑在一起，" << endl;
    cout << "你决定在哪执行它，比 async 灵活一点，比 promise 方便一点。" << endl;
}

// ============================================================
// 六、异常在 future 中的传递
// ============================================================
// 老师傅强调：
// future 有个很重要的特性：异常也能传递！
// 不管是 async、packaged_task 还是 promise，
// 只要任务执行时抛了异常，异常都会被存到 future 里，
// 你调用 get() 的时候，异常就会在你这里重新抛出来。
//
// 这太重要了！不然工作线程里抛个异常，程序直接崩了，
// 你连咋死的都不知道。
// ============================================================

void demo_future_exception() {
    cout << "\n=== 演示7：future 中的异常传递 ===" << endl;

    // async 中的异常
    cout << "--- async 中的异常 ---" << endl;
    future<void> f1 = async(launch::async, []() {
        throw runtime_error("async 任务出问题了！");
    });

    try {
        f1.get();
    } catch (const exception& e) {
        cout << "捕获到异常: " << e.what() << endl;
    }

    // packaged_task 中的异常
    cout << "\n--- packaged_task 中的异常 ---" << endl;
    packaged_task<void()> task([]() {
        throw runtime_error("packaged_task 出问题了！");
    });
    future<void> f2 = task.get_future();
    thread t(move(task));
    t.join();

    try {
        f2.get();
    } catch (const exception& e) {
        cout << "捕获到异常: " << e.what() << endl;
    }

    cout << "\n看到没？不管哪种方式，异常都会被安全地传递到 get() 调用处。" << endl;
    cout << "这比自己用全局变量传异常安全多了。" << endl;
}

// ============================================================
// 七、std::shared_future - 多线程等同一个结果
// ============================================================
// 老师傅讲场景：
// 普通的 future 是"一次性"的——只有一个人能调用 get()，
// 而且只能调用一次（调用后 future 就废了）。
//
// 但有时候，好几个线程都需要同一个异步任务的结果。
// 比如：一个线程下载配置文件，好几个线程都要用这个配置。
//
// 这时候就需要 shared_future：
//   - 可以有多个副本
//   - 每个副本都能调用 get()
//   - 结果只计算一次，所有人共享
//
// 比喻：
//   future 就像外卖，只能一个人吃，吃完就没了。
//   shared_future 就像一大锅汤，盛好几碗，大家都能喝。
// ============================================================

void demo_shared_future() {
    cout << "\n=== 演示8：std::shared_future 多线程共享结果 ===" << endl;

    // 用 async 启动一个任务，然后把 future 转成 shared_future
    shared_future<int> sf = async(launch::async, []() {
        cout << "  [配置加载] 正在加载配置文件..." << endl;
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "  [配置加载] 加载完成" << endl;
        return 42;  // 假设配置里有个值是42
    }).share();  // share() 把 future 变成 shared_future

    // 多个线程都要用这个配置
    vector<thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([sf, i]() {  // 每个线程都拷贝一份 shared_future
            cout << "线程" << i << "：等待配置..." << endl;
            int config_value = sf.get();   // 每个人都能 get()
            cout << "线程" << i << "：拿到配置值 = " << config_value
                 << "，开始干活..." << endl;
            this_thread::sleep_for(chrono::milliseconds(100));
            cout << "线程" << i << "：干完了" << endl;
        });
    }

    for (auto& t : threads) t.join();

    cout << "\nshared_future 让多个线程可以等待同一个异步结果，" << endl;
    cout << "结果只计算一次，大家共享，很方便。" << endl;
}

// ============================================================
// 八、三种异步方式对比总结
// ============================================================
// 老师傅画个表：
//
// 方式          | 方便程度 | 灵活度 | 适用场景
// --------------|---------|-------|-------------------------
// std::async    | 最方便   | 最低  | 简单的异步任务，快速上手
// packaged_task | 中等     | 中等  | 需要控制任务执行时机
// promise       | 最麻烦   | 最高  | 完全自定义的异步逻辑
//
// 老师傅的经验：
//   - 能用 async 就用 async，写起来最省事
//   - 需要把任务存起来以后执行，用 packaged_task
//   - async 和 packaged_task 都满足不了，再用 promise
// ============================================================

void demo_three_ways_compared() {
    cout << "\n=== 演示9：三种异步方式对比 ===" << endl;

    int n = 10;

    // 方式1：async
    cout << "方式1 - std::async:" << endl;
    future<int> f1 = async(launch::async, calculate_fib, n);
    cout << "  fib(" << n << ") = " << f1.get() << endl;

    // 方式2：packaged_task + thread
    cout << "方式2 - std::packaged_task:" << endl;
    packaged_task<int(int)> task(calculate_fib);
    future<int> f2 = task.get_future();
    thread(move(task), n).detach();  // detach 只是演示，实际项目要小心
    cout << "  fib(" << n << ") = " << f2.get() << endl;

    // 方式3：promise + thread
    cout << "方式3 - std::promise:" << endl;
    promise<int> p;
    future<int> f3 = p.get_future();
    thread([p = move(p), n]() mutable {
        p.set_value(calculate_fib(n));
    }).detach();
    cout << "  fib(" << n << ") = " << f3.get() << endl;

    cout << "\n三种方式都能拿到结果，但灵活度和方便程度不一样。" << endl;
    cout << "简单任务用 async，复杂场景才考虑另外两种。" << endl;
}

// ============================================================
// 九、异步任务编排 - 并行任务的组合
// ============================================================
// 老师傅讲场景：
// 实际项目中，异步任务往往不是孤立的，而是有依赖关系的。
// 比如：
//   - 先并行下载A和B，都下载完了再处理
//   - 先查数据库，查到了再调接口
//
// C++17 标准库里没有 when_all/when_any。
// 截至 C++23 也没有；C++26 的 std::execution（P2300）才提供 when_all/when_any，
// 但编译器/标准库支持还不普及，所以这里先自己简单模拟，理解思路就行。
//
// 实际项目中可以用：
//   - Folly 的 Future 库（Facebook的，功能强大）
//   - 自己封装
// ============================================================

// 简单模拟 when_all：等待所有 future 就绪
template <typename T>
vector<T> when_all(vector<future<T>>& futures) {
    vector<T> results;
    for (auto& f : futures) {
        results.push_back(f.get());
    }
    return results;
}

void demo_parallel_orchestration() {
    cout << "\n=== 演示10：并行任务编排 ===" << endl;

    // 场景：并行下载3个文件，都下完了再处理
    cout << "场景：并行下载3个数据文件，都完成后汇总" << endl;

    auto download = [](const string& name) -> string {
        cout << "  开始下载: " << name << endl;
        // 用名字长度做固定延时，避免多线程同时调用 rand()（C 的 rand 不保证线程安全）
        this_thread::sleep_for(chrono::milliseconds(200 + static_cast<int>(name.size()) * 50));
        cout << "  下载完成: " << name << endl;
        return name + " 的内容";
    };

    // 启动3个并行下载
    vector<future<string>> downloads;
    downloads.push_back(async(launch::async, download, "文件A"));
    downloads.push_back(async(launch::async, download, "文件B"));
    downloads.push_back(async(launch::async, download, "文件C"));

    // 等待所有下载完成
    cout << "主线程：等待所有下载完成..." << endl;
    vector<string> results = when_all(downloads);

    cout << "所有下载完成，汇总结果：" << endl;
    for (const auto& r : results) {
        cout << "  - " << r << endl;
    }

    cout << "\n这就是最简单的并行任务编排：" << endl;
    cout << "多个任务并行执行，全部完成后再进行下一步。" << endl;
    cout << "标准库没有 std::when_all/when_any；C++26 的 std::execution" << endl;
    cout << "才提供 when_all/when_any，且支持尚不普及。" << endl;
}

// ============================================================
// 十、实际应用：并行快速排序
// ============================================================
// 老师傅说：
// 用 future 实现并行快排，是个经典例子。
// 思路很简单：选一个基准值，把数组分成左右两部分，
// 左边异步排，右边同步排（或者也异步），然后合并。
//
// 注意：这只是演示思想，实际性能不一定更好，
// 因为小任务开线程的开销可能大于收益。
// 真正的并行排序应该用 Intel TBB 之类的库。
// ============================================================

void quick_sort(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int pivot = arr[left + (right - left) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }

    // 小任务就别异步了，直接排（避免线程过多）
    if (right - left < 1000) {
        quick_sort(arr, left, j);
        quick_sort(arr, i, right);
    } else {
        // 左边异步排，右边同步排
        auto f = async(launch::async, [&arr, left, j]() {
            quick_sort(arr, left, j);
        });
        quick_sort(arr, i, right);
        f.get();  // 等左边排完
    }
}

void demo_parallel_quick_sort() {
    cout << "\n=== 演示11：并行快速排序（future 实战） ===" << endl;

    const int N = 50000;
    vector<int> arr(N);
    for (int i = 0; i < N; ++i) {
        arr[i] = rand() % 10000;
    }

    vector<int> arr2 = arr;  // 复制一份

    // 基准：单线程、高度优化的 std::sort（不是串行版 quick_sort）
    auto start = chrono::steady_clock::now();
    sort(arr.begin(), arr.end());
    auto end = chrono::steady_clock::now();
    auto std_time = chrono::duration<double, milli>(end - start).count();
    cout << "std::sort（单线程基准）耗时: " << std_time << "ms" << endl;

    // 并行快排
    start = chrono::steady_clock::now();
    quick_sort(arr2, 0, arr2.size() - 1);
    end = chrono::steady_clock::now();
    auto parallel_time = chrono::duration<double, milli>(end - start).count();
    cout << "并行快排 耗时: " << parallel_time << "ms" << endl;

    // 验证
    bool correct = (arr == arr2);
    cout << "排序结果" << (correct ? "正确" : "错误！") << endl;
    cout << "注意：这不是同算法的严格对比——基准是单线程的 std::sort，" << endl;
    cout << "      另一边是手写 quick_sort + async。线程创建、数据分布、" << endl;
    cout << "      重复元素都会影响结果，不能仅凭这两行断言谁更快。" << endl;
    cout << "      数组较小时，线程开销通常会让并行版本更慢。" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "============================================================" << endl;
    cout << "  并发编程进阶06：异步编程 future/promise" << endl;
    cout << "============================================================" << endl;

    demo_async_basic();           // std::async 基本用法
    demo_async_launch_policy();   // launch 策略对比
    demo_future_wait();           // future 等待方式
    demo_promise();               // promise 手动设置结果
    demo_promise_exception();     // promise 设置异常
    demo_packaged_task();         // packaged_task 打包任务
    demo_future_exception();      // 异常传递
    demo_shared_future();         // shared_future
    demo_three_ways_compared();   // 三种方式对比
    demo_parallel_orchestration(); // 并行任务编排
    demo_parallel_quick_sort();   // 并行快排实战

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "徒弟啊，future 和 promise 是C++异步编程的基石，" << endl;
    cout << "写小工具、做并行计算特别方便。" << endl;
    cout << "" << endl;
    cout << "1. 三兄弟的选择：" << endl;
    cout << "   - 简单异步任务，直接用 async，最省事" << endl;
    cout << "   - 需要把任务存起来以后执行，用 packaged_task" << endl;
    cout << "   - 需要完全自定义结果设置时机，用 promise" << endl;
    cout << "   记住：async 最常用，promise 最灵活，packaged_task 中间态。" << endl;
    cout << "" << endl;
    cout << "2. future 的几个关键点：" << endl;
    cout << "   - get() 只能调用一次，调用后 future 就失效了" << endl;
    cout << "   - wait_for/wait_until 可以做超时控制，很实用" << endl;
    cout << "   - 异常会被安全传递，在 get() 处抛出" << endl;
    cout << "   - 多个线程等同一个结果，用 shared_future" << endl;
    cout << "" << endl;
    cout << "3. async 是最简单的并发入门，写小工具特别方便。" << endl;
    cout << "   比如你要批量处理一堆文件，用 async 开几个并行任务，" << endl;
    cout << "   比手动管 thread 舒服多了。" << endl;
    cout << "" << endl;
    cout << "4. 但 async 也不是万能的：" << endl;
    cout << "   - 大量小任务用 async 不如线程池（线程创建有开销）" << endl;
    cout << "   - future 没有 .then() 链式调用（C++20 也没有加进来）" << endl;
    cout << "   - 没法取消任务（future 没有 cancel）" << endl;
    cout << "" << endl;
    cout << "5. C++20 及以后带来了更好用的东西：" << endl;
    cout << "   C++20 有 std::jthread（可中断线程）和协程（coroutine）；" << endl;
    cout << "   C++26 的 std::execution 才提供 when_all/when_any 和发送者/接收者模型。" << endl;
    cout << "   但底层的思想是相通的。" << endl;
    cout << "   先把 future/promise 吃透，学新东西就快了。" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习" << endl;
    cout << "============================================================" << endl;
    cout << "1. 用 std::async 实现一个并行版的 std::transform：" << endl;
    cout << "   把一个大数组的每个元素都做某个转换，并行处理。" << endl;
    cout << "   （提示：把数组分成几块，每块一个 async 任务）" << endl;
    cout << "" << endl;
    cout << "2. 用 promise 实现一个简单的“一次性事件”：" << endl;
    cout << "   一个线程等待事件，另一个线程触发事件。" << endl;
    cout << "   （类似 condition_variable，但只能触发一次）" << endl;
    cout << "" << endl;
    cout << "3. 实现一个简单的 when_any 函数：" << endl;
    cout << "   给定一组 future，返回第一个就绪的结果。" << endl;
    cout << "   （提示：可以轮询每个 future 的 wait_for(0)）" << endl;
    cout << "" << endl;
    cout << "4. 思考：future 的 get() 为什么只能调用一次？" << endl;
    cout << "   如果想多次获取结果怎么办？（提示：shared_future）" << endl;
    cout << "   为什么 future 设计成“一次性”的？" << endl;
    cout << "" << endl;
    cout << "5. 查一下 std::async 的默认启动策略：" << endl;
    cout << "   如果你不指定 launch::async 或 launch::deferred，" << endl;
    cout << "   它到底会怎么选择？这可能导致什么坑？" << endl;

    return 0;
}
