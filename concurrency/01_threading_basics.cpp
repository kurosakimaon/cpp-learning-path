// ============================================================
// 01_threading_basics.cpp - 线程基础（老师傅带徒弟系列）
// 编译命令：g++ -std=c++17 01_threading_basics.cpp -o demo1 -pthread
// ============================================================
//
// 【老师傅开场白】
// 徒弟啊，今天咱们开讲并发编程的第一课——线程。
// 你就把进程想象成一个工厂车间，线程呢？就是车间里的一条条流水线。
// 一个车间（进程）里可以有多条流水线（线程），它们共享车间里的
// 原材料、工具、仓库（也就是进程的内存空间），但每条流水线各干各的活。
//
// 为啥要用多线程？说白了就是"人多力量大"，但人多了也容易乱——
// 这就是咱们后面要讲的同步、互斥、死锁等等问题的根源。
// 先别急，咱们一步一步来，今天先把线程的基本玩法搞明白。

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>

using namespace std;

// ============================================================
// 一、线程的创建方式（四种常见写法）
// ============================================================

// 方式1：普通函数（函数指针）
void printHello(const string& name, int times) {
    for (int i = 0; i < times; ++i) {
        cout << "你好，" << name << "！（第" << i + 1 << "次）" << endl;
    }
}

// 方式3：仿函数（函数对象）
struct Worker {
    int id;
    void operator()(int taskCount) const {
        cout << "工人" << id << "号开始干活，任务数：" << taskCount << endl;
        for (int i = 0; i < taskCount; ++i) {
            // 假装在干活，睡 10 毫秒
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        cout << "工人" << id << "号收工了！" << endl;
    }
};

// 方式4：成员函数
class Chef {
public:
    string name;
    void cook(string dish, int portions) {
        cout << "厨师" << name << "开始做" << dish << "，共" << portions << "份" << endl;
        for (int i = 0; i < portions; ++i) {
            this_thread::sleep_for(chrono::milliseconds(20));
            cout << "  " << dish << "第" << i + 1 << "份出锅~" << endl;
        }
    }
};

// ============================================================
// 九、ThreadGuard 类（RAII 思想管理线程）
// ============================================================
// 【老师傅的比喻】
// 你去澡堂子洗澡，进去的时候拿钥匙（创建线程），走的时候得还钥匙（join）。
// 要是你洗完澡直接溜了，钥匙没还——那澡堂子就得找你麻烦（程序崩溃）。
// RAII 就是让"钥匙"自动还：对象构造时拿钥匙，析构时自动还。
class ThreadGuard {
private:
    thread& t;  // 持有线程的引用
public:
    // 构造时接收一个线程引用
    explicit ThreadGuard(thread& _t) : t(_t) {}

    // 析构时自动 join（前提是线程还能 join）
    ~ThreadGuard() {
        if (t.joinable()) {
            cout << "[ThreadGuard] 自动帮你 join 了线程，放心吧~" << endl;
            t.join();
        }
    }

    // 禁止拷贝和赋值，防止重复析构
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

// ============================================================
// 各个示例函数
// ============================================================

// 示例1：线程的四种创建方式
void demo1_thread_creation() {
    cout << "\n========== 示例1：线程的四种创建方式 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "创建线程就像安排人去干活，有好几种派活的方式：" << endl;
    cout << "  1. 直接告诉一个函数去干（函数指针）" << endl;
    cout << "  2. 写一段临时代码扔过去（lambda）" << endl;
    cout << "  3. 让一个带状态的对象去干（仿函数）" << endl;
    cout << "  4. 让某个对象的某个方法去干（成员函数）" << endl;
    cout << endl;

    // 方式1：函数指针
    thread t1(printHello, "张三", 3);

    // 方式2：lambda 表达式（最灵活，推荐！）
    thread t2([](string msg) {
        cout << "Lambda 说：" << msg << endl;
    }, "我是 lambda 线程");

    // 方式3：仿函数
    Worker worker{101};
    thread t3(worker, 5);  // 注意：worker 会被拷贝进线程

    // 方式4：成员函数
    Chef chef{"老李"};
    // 第一个参数是成员函数指针，第二个是对象指针，后面是函数参数
    thread t4(&Chef::cook, &chef, "红烧肉", 3);

    // 等所有线程干完活
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    cout << "\n所有线程都跑完了！" << endl;
}

// 示例2：join vs detach（必须掌握！）
void demo2_join_vs_detach() {
    cout << "\n========== 示例2：join() vs detach() ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "join() 就是：主线程站在这儿等，等子线程干完活再往下走。" << endl;
    cout << "detach() 就是：主线程说'你自己玩吧'，然后各走各的，子线程在后台跑。" << endl;
    cout << "注意！detach 之后，线程就不受你管了，程序退出时它可能还在跑。" << endl;
    cout << "初学者老老实实先用 join，detach 水很深，把握不住。" << endl;
    cout << endl;

    // join 的例子
    thread t1([]() {
        this_thread::sleep_for(chrono::milliseconds(50));
        cout << "子线程A：我干完了，等我呢吧？" << endl;
    });
    cout << "主线程：等 t1 干完..." << endl;
    t1.join();  // 主线程在这里等
    cout << "主线程：好，t1 干完了，继续走。" << endl;

    // detach 的例子
    thread t2([]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        // 注意：detach 的线程可能在 main 结束后还在跑，输出可能看不到
        cout << "子线程B：我在后台默默干活..." << endl;
    });
    cout << "主线程：t2 你自己玩吧，我不等了。" << endl;
    t2.detach();  // 分离，线程在后台运行

    // 小等一下，让 detach 的线程有机会输出
    this_thread::sleep_for(chrono::milliseconds(200));
    cout << "主线程：我也快完事了。" << endl;
}

// 示例3：线程 ID 和硬件并发数
void demo3_thread_id_and_hardware() {
    cout << "\n========== 示例3：线程ID 与硬件并发数 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "每个线程都有一个身份证号，叫线程 ID，可以用 get_id() 查看。" << endl;
    cout << "this_thread::get_id() 能拿到当前线程的 ID。" << endl;
    cout << "hardware_concurrency() 告诉你 CPU 大概能同时跑几个线程（核心数/线程数）。" << endl;
    cout << "记住：线程不是越多越好，太多了反而会因为切换开销变慢。" << endl;
    cout << endl;

    cout << "主线程 ID：" << this_thread::get_id() << endl;
    cout << "CPU 支持的硬件并发数：" << thread::hardware_concurrency() << endl;
    cout << "（通常是 CPU 的逻辑核心数，比如 8 核 16 线程会显示 16；"
            "但标准只把它当提示，拿不到信息时返回 0，别当硬上限用）" << endl;

    thread t([]() {
        cout << "子线程 ID：" << this_thread::get_id() << endl;
    });
    cout << "t 的 ID：" << t.get_id() << endl;
    t.join();
}

// 示例4：线程函数的参数传递
void demo4_parameter_passing() {
    cout << "\n========== 示例4：线程参数传递 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "给线程传参数，默认是拷贝到线程里的。要传引用得用 std::ref。" << endl;
    cout << "还有移动语义，用 std::move 把资源'转移'给线程，避免拷贝。" << endl;
    cout << "坑点：传指针或引用的时候，一定要保证对象在线程运行期间还活着！" << endl;
    cout << endl;

    // 值传递（默认行为，最安全）
    int x = 10;
    thread t1([](int val) {
        val = 100;  // 改的是拷贝，不影响外面
        cout << "值传递：线程里 val = " << val << endl;
    }, x);
    t1.join();
    cout << "值传递：外面 x = " << x << "（没变，因为是拷贝）" << endl;

    // 引用传递（要用 std::ref 包装）
    int y = 10;
    thread t2([](int& val) {
        val = 200;
        cout << "引用传递：线程里 val = " << val << endl;
    }, ref(y));  // 注意这里的 ref！
    t2.join();
    cout << "引用传递：外面 y = " << y << "（变了，因为是引用）" << endl;

    // 移动语义（适合大对象或不可拷贝的对象）
    vector<int> bigData(1000000, 42);
    cout << "移动前：bigData 大小 = " << bigData.size() << endl;
    thread t3([](vector<int> data) {
        cout << "移动后：线程里 data 大小 = " << data.size() << endl;
    }, move(bigData));  // 把 bigData 移动进线程，外面的 bigData 就空了
    t3.join();
    cout << "移动后：外面 bigData 大小 = " << bigData.size() << "（被移走了）" << endl;
}

// 示例5：线程所有权转移
void demo5_thread_ownership() {
    cout << "\n========== 示例5：线程所有权转移 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "std::thread 是不可拷贝的，就像一个独苗，只能有一个主人。" << endl;
    cout << "但可以通过 std::move 转移所有权，就像交接工作一样。" << endl;
    cout << "也可以把线程放到容器里（比如 vector），前提是用 move。" << endl;
    cout << endl;

    thread t1([]() {
        cout << "线程：我一开始属于 t1" << endl;
    });

    cout << "t1 joinable? " << (t1.joinable() ? "是" : "否") << endl;

    // 把 t1 的所有权转移给 t2
    thread t2 = move(t1);
    cout << "move 之后..." << endl;
    cout << "t1 joinable? " << (t1.joinable() ? "是" : "否") << "（已经空了）" << endl;
    cout << "t2 joinable? " << (t2.joinable() ? "是" : "否") << "（接过来了）" << endl;

    t2.join();

    // 把线程放进 vector
    vector<thread> pool;
    for (int i = 0; i < 3; ++i) {
        pool.emplace_back([i]() {
            cout << "线程池里的线程 " << i << " 跑起来了" << endl;
        });
    }
    // 等所有线程跑完
    for (auto& t : pool) {
        t.join();
    }
    cout << "线程池里的线程都跑完了" << endl;
}

// 示例6：异常安全与 ThreadGuard
void demo6_exception_safety() {
    cout << "\n========== 示例6：异常安全与 ThreadGuard ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "想象一下：你创建了一个线程，还没来得及 join，中间代码抛异常了——" << endl;
    cout << "那线程对象就被意外销毁了，程序直接崩溃（std::terminate）！" << endl;
    cout << "怎么办？用 RAII 思想——写一个 ThreadGuard 类，它析构时自动 join。" << endl;
    cout << "不管是正常走完还是抛异常，析构函数都会被调用，线程一定能被 join 上。" << endl;
    cout << endl;

    try {
        thread t([]() {
            cout << "子线程：我在干活呢..." << endl;
            this_thread::sleep_for(chrono::milliseconds(30));
            cout << "子线程：干完了！" << endl;
        });

        // 用 ThreadGuard 包裹，保证即使抛异常也能 join
        ThreadGuard guard(t);

        // 假装中间出了点问题，抛了个异常
        cout << "主线程：哎呀，出异常了！" << endl;
        throw runtime_error("不小心出错了");

        // 下面这句永远执行不到，但 ThreadGuard 会替我们 join
        // t.join();

    } catch (const exception& e) {
        cout << "捕获到异常：" << e.what() << endl;
        cout << "但是！线程已经被 ThreadGuard 安全地 join 了，没有崩溃。" << endl;
    }
}

// ============================================================
// 老师傅的话
// ============================================================
void laoShiFuDeHua() {
    cout << "\n========== 老师傅的话 ==========" << endl;
    cout << "徒弟啊，今天讲的东西有点多，给你划几个重点：" << endl;
    cout << endl;
    cout << "1. 线程不是越多越好" << endl;
    cout << "   很多新手一上来就开几百个线程，觉得越多越快。大错特错！" << endl;
    cout << "   线程切换是有开销的，开多了反而慢。一般 CPU 密集型任务，" << endl;
    cout << "   线程数跟核心数差不多就行；IO 密集型可以多开一些。" << endl;
    cout << endl;
    cout << "2. detach 要慎用" << endl;
    cout << "   detach 就是把线程'放飞自我'，你再也管不了它了。" << endl;
    cout << "   程序退出时 detach 的线程会被直接杀掉，可能资源没释放。" << endl;
    cout << "   除非你明确知道自己在干啥，否则老老实实 join。" << endl;
    cout << endl;
    cout << "3. 用 RAII 管理线程" << endl;
    cout << "   跟智能指针管理内存一个道理，用 ThreadGuard 或者更高级的封装，" << endl;
    cout << "   让线程自动 join，省心又安全。" << endl;
    cout << endl;
    cout << "4. 参数传递要小心" << endl;
    cout << "   传引用用 std::ref，传指针要保证指针对象生命周期够长。" << endl;
    cout << "   拿不准就值传递，虽然慢点，但安全。" << endl;
}

// ============================================================
// 小练习
// ============================================================
void xiaoLianXi() {
    cout << "\n========== 小练习 ==========" << endl;
    cout << "学完了？来做几道题巩固一下：" << endl;
    cout << endl;
    cout << "1. 写一个程序，创建 5 个线程，每个线程打印自己的编号（1~5）。" << endl;
    cout << "   思考：输出顺序是固定的吗？为什么？" << endl;
    cout << endl;
    cout << "2. 改造 ThreadGuard，让它支持 detach 模式（构造时选择 join 还是 detach）。" << endl;
    cout << endl;
    cout << "3. 写一个函数，接收一个 vector<int>，开两个线程：" << endl;
    cout << "   一个算前半部分的和，一个算后半部分的和，最后加起来。" << endl;
    cout << "   （提示：用引用传递把结果带回来）" << endl;
    cout << endl;
    cout << "4. 思考题：为什么 std::thread 不允许拷贝？" << endl;
    cout << "   （提示：想想如果能拷贝会发生什么）" << endl;
    cout << endl;
    cout << "答案就在代码里多琢磨琢磨，不懂的地方多动手试！" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "================================================" << endl;
    cout << "  并发编程第一课：线程基础（老师傅带徒弟）" << endl;
    cout << "================================================" << endl;

    demo1_thread_creation();     // 线程的四种创建方式
    demo2_join_vs_detach();      // join vs detach
    demo3_thread_id_and_hardware(); // 线程ID与硬件并发数
    demo4_parameter_passing();   // 参数传递
    demo5_thread_ownership();    // 所有权转移
    demo6_exception_safety();    // 异常安全与ThreadGuard

    laoShiFuDeHua();  // 老师傅的话
    xiaoLianXi();     // 小练习

    cout << "\n================================================" << endl;
    cout << "  第一课完毕，下节课咱们讲同步原语！" << endl;
    cout << "================================================" << endl;

    return 0;
}
