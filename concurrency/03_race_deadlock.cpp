// ============================================================
// 03_race_deadlock.cpp - 竞态条件与死锁（老师傅带徒弟系列）
// 编译命令：g++ -std=c++17 03_race_deadlock.cpp -o demo3 -pthread
// ============================================================
//
// 【老师傅开场白】
// 徒弟啊，前两节课咱们学了线程和同步原语，今天咱们聊聊并发编程里
// 最让人头疼的两类 bug：竞态条件和死锁。
//
// 这俩东西有个共同点：它们不一定每次都出现。你跑100次可能99次都好好的，
// 就那1次出问题，而且一出问题就难找。所以老师傅我常说：
// "并发 bug 最难调试，写代码的时候就得想清楚，别等出了问题再查。"

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <climits>
#include <stdexcept>

using namespace std;

// ============================================================
// 一、什么是竞态条件
// ============================================================
// 【老师傅的比喻】
// 想象两个人同时去银行取同一账户里的钱：
//   账户里有 1000 块
//   A 查余额：1000，取 800
//   B 查余额：1000（A还没来得及改），也取 800
//   结果两人各取走 800，总共取了 1600，账户还剩 200
//   银行亏了 400！
// 这就是竞态条件：程序的行为取决于线程的执行顺序（运气），
// 顺序不对结果就错了。

// 有 bug 的计数器（竞态条件演示）
// volatile 只能阻止编译器把循环优化掉，让竞态更容易被观察到；
// 它不提供原子性，也不是线程同步手段。
volatile int buggyCounter = 0;

void buggyIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        // 这一行看起来简单，但实际上是三步：
        // 1. 从内存读到寄存器  2. 寄存器加一  3. 写回内存
        // 两个线程在这三步之间穿插执行，结果就错了
        buggyCounter++;
    }
}

// ============================================================
// 二、数据竞争的直观展示
// ============================================================
// 故意写一个有明显 bug 的程序，让你亲眼看看数据竞争的后果。
// 我们用两个线程同时对一个变量做大量 ++ 操作，看看结果差多少。

void demo_race_condition() {
    cout << "\n========== 示例1：竞态条件演示 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "两个线程各加 1000 万次，理论结果应该是 2000 万。" << endl;
    cout << "但不加锁的话...咱们跑一下看看。" << endl;
    cout << "（可能要等几秒，耐心点）" << endl;
    cout << endl;

    const int TIMES = 10000000;  // 1000 万次，效果明显

    buggyCounter = 0;

    thread t1(buggyIncrement, TIMES);
    thread t2(buggyIncrement, TIMES);

    t1.join();
    t2.join();

    int expected = 2 * TIMES;
    cout << "实际结果：" << buggyCounter << endl;
    cout << "预期结果：" << expected << endl;
    cout << "差了多少：" << (expected - buggyCounter) << endl;
    cout << "错误率：" << (expected - buggyCounter) * 100.0 / expected << "%" << endl;
    cout << endl;
    if (buggyCounter != expected) {
        cout << "看到没？差了不老少呢。而且每次跑结果可能还不一样，" << endl;
        cout << "这就是竞态条件的'魅力'——不稳定，难复现。" << endl;
    } else {
        cout << "这次碰巧对了，但这行代码仍然是数据竞争（UB），" << endl;
        cout << "换个编译器、优化级别或机器就可能出错。" << endl;
        cout << "这就是竞态条件'难复现'的可怕之处。" << endl;
    }
}

// ============================================================
// 三、用互斥锁修复竞态条件
// ============================================================

int fixedCounter = 0;
mutex counterMutex;

void fixedIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        lock_guard<mutex> lock(counterMutex);
        fixedCounter++;
    }
}

// 也可以用原子变量，更高效
atomic<int> atomicCounter(0);

void atomicIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        atomicCounter++;  // 原子操作，线程安全
    }
}

void demo_fix_race_condition() {
    cout << "\n========== 示例2：修复竞态条件 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "修复方法有两种思路：" << endl;
    cout << "  1. 加互斥锁：保证同一时间只有一个线程操作共享数据" << endl;
    cout << "  2. 用原子变量：CPU 提供的原子指令，硬件层面保证不可分割" << endl;
    cout << "简单的计数器这种，用 atomic 更高效；复杂的操作还是得用锁。" << endl;
    cout << endl;

    const int TIMES = 1000000;  // 少点，不然等太久

    // 互斥锁版本
    fixedCounter = 0;
    auto start1 = chrono::high_resolution_clock::now();
    thread t1(fixedIncrement, TIMES);
    thread t2(fixedIncrement, TIMES);
    t1.join();
    t2.join();
    auto end1 = chrono::high_resolution_clock::now();
    cout << "互斥锁版本：结果 = " << fixedCounter
         << "，耗时 = " << chrono::duration_cast<chrono::milliseconds>(end1 - start1).count() << "ms" << endl;

    // 原子变量版本
    atomicCounter = 0;
    auto start2 = chrono::high_resolution_clock::now();
    thread t3(atomicIncrement, TIMES);
    thread t4(atomicIncrement, TIMES);
    t3.join();
    t4.join();
    auto end2 = chrono::high_resolution_clock::now();
    cout << "原子变量版本：结果 = " << atomicCounter.load()
         << "，耗时 = " << chrono::duration_cast<chrono::milliseconds>(end2 - start2).count() << "ms" << endl;

    cout << "\n在这个低竞争的单变量计数例子里，原子变量更快。" << endl;
    cout << "但别把它当成万能结论：高竞争时原子 RMW 会争用同一缓存行，" << endl;
    cout << "未必比 mutex 快；而且原子只适合保护单个变量，" << endl;
    cout << "多个变量要保持一致时仍然得用锁。" << endl;
}

// ============================================================
// 四、什么是死锁
// ============================================================
// 【老师傅的比喻——哲学家就餐问题】
// 五个哲学家围坐一桌，每人面前一碗饭，每两人之间一根筷子。
// 规则：
//   - 哲学家要么思考，要么吃饭
//   - 吃饭需要两根筷子（左手边和右手边的）
//   - 每次只能拿一根筷子
//
// 死锁场景：
//   五个哲学家同时拿起左手的筷子，然后都等右手的筷子...
//   结果每个人手里都拿着一根，等着另一根，永远等不到——死锁了！
//
// 死锁的四个必要条件（缺一个都死不了）：
//   1. 互斥条件：资源不能共享，只能一个人用（筷子一次只能一个人拿）
//   2. 占有并等待：拿着一个资源，等另一个（拿了左筷子等右筷子）
//   3. 不可剥夺：资源不能被抢走，只能主动释放（筷子不能抢）
//   4. 循环等待：A 等 B，B 等 C，C 等 A（形成一个环）

// 简化的死锁演示：两个线程，两把锁，加锁顺序相反
mutex fork1;  // 左筷子
mutex fork2;  // 右筷子

// 哲学家 A：先拿左筷子，再拿右筷子
void philosopherA() {
    cout << "哲学家A：我先拿左筷子..." << endl;
    lock_guard<mutex> left(fork1);  // 拿左筷子
    this_thread::sleep_for(chrono::milliseconds(10));  // 思考一下
    cout << "哲学家A：拿到左的了，等右筷子..." << endl;
    lock_guard<mutex> right(fork2);  // 等右筷子（可能死锁）
    cout << "哲学家A：两根都拿到了，开吃！" << endl;
}

// 哲学家 B：先拿右筷子，再拿左筷子（顺序反了！）
void philosopherB() {
    cout << "哲学家B：我先拿右筷子..." << endl;
    lock_guard<mutex> right(fork2);  // 拿右筷子
    this_thread::sleep_for(chrono::milliseconds(10));  // 思考一下
    cout << "哲学家B：拿到右的了，等左筷子..." << endl;
    lock_guard<mutex> left(fork1);  // 等左筷子（可能死锁）
    cout << "哲学家B：两根都拿到了，开吃！" << endl;
}

// ============================================================
// 五、死锁演示
// ============================================================
// 注意：这个示例可能会卡住（死锁），所以我们用一个"侦探"线程来检测。
// 为了不真的卡死程序，我们用 timed_mutex 来做演示，这样能看出"等不到"的效果。

timed_mutex tFork1, tFork2;

void demo_deadlock() {
    cout << "\n========== 示例3：死锁演示 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "两个哲学家，两根筷子，拿的顺序相反。" << endl;
    cout << "如果运气不好，就会你等我我等你，谁也吃不上——这就是死锁。" << endl;
    cout << "为了不真的卡死程序，我用了 timed_mutex，等 500ms 等不到就放弃。" << endl;
    cout << "注意：这是“超时模拟”，不是真正的死锁——真正的死锁会无限期等下去，" << endl;
    cout << "把下面的 try_lock_for 换成 lock() 才会真的卡死。" << endl;
    cout << endl;

    // 哲学家 A：先 1 后 2
    thread ta([&]() {
        tFork1.lock();
        cout << "哲学家A：拿到左筷子了" << endl;
        this_thread::sleep_for(chrono::milliseconds(20));  // 增加死锁概率

        if (tFork2.try_lock_for(chrono::milliseconds(500))) {
            cout << "哲学家A：两根都拿到了，吃饭！" << endl;
            tFork2.unlock();
        } else {
            cout << "哲学家A：右筷子等不到，超时放弃（模拟死锁）" << endl;
        }
        tFork1.unlock();
    });

    // 哲学家 B：先 2 后 1（顺序反了）
    thread tb([&]() {
        tFork2.lock();
        cout << "哲学家B：拿到右筷子了" << endl;
        this_thread::sleep_for(chrono::milliseconds(20));  // 增加死锁概率

        if (tFork1.try_lock_for(chrono::milliseconds(500))) {
            cout << "哲学家B：两根都拿到了，吃饭！" << endl;
            tFork1.unlock();
        } else {
            cout << "哲学家B：左筷子等不到，超时放弃（模拟死锁）" << endl;
        }
        tFork2.unlock();
    });

    ta.join();
    tb.join();

    cout << "\n看到没？两个都没吃上饭——这里用超时模拟了循环等待。" << endl;
    cout << "真正的死锁不会超时，而是永久阻塞；实际程序里一旦死锁，" << endl;
    cout << "相关线程就再也无法推进。" << endl;
}

// ============================================================
// 六、如何避免死锁
// ============================================================

// 方法1：按固定顺序加锁
// 所有线程都按同样的顺序拿锁，就不会形成循环等待。
// 比如：永远先拿编号小的筷子，再拿编号大的。
void method1_fixed_order() {
    cout << "\n--- 方法1：按固定顺序加锁 ---" << endl;
    cout << "思路：所有人都按'从小到大'的顺序拿筷子，" << endl;
    cout << "      就不会出现你等我我等你的环了。" << endl;

    mutex m1, m2;  // 假设 m1 < m2（按地址或编号排序）

    thread ta([&]() {
        // A 也先拿 m1 再拿 m2
        lock_guard<mutex> lk1(m1);
        this_thread::sleep_for(chrono::milliseconds(5));
        lock_guard<mutex> lk2(m2);
        cout << "  哲学家A：按顺序拿，吃上饭了！" << endl;
    });

    thread tb([&]() {
        // B 也先拿 m1 再拿 m2（跟 A 一样的顺序）
        lock_guard<mutex> lk1(m1);
        this_thread::sleep_for(chrono::milliseconds(5));
        lock_guard<mutex> lk2(m2);
        cout << "  哲学家B：按顺序拿，吃上饭了！" << endl;
    });

    ta.join();
    tb.join();
    cout << "  都吃上了，完美！" << endl;
}

// 方法2：用 std::lock / scoped_lock 同时加锁
// 一次性把所有需要的锁都拿到，要么全拿，要么不拿。
void method2_scoped_lock() {
    cout << "\n--- 方法2：scoped_lock 同时加锁 ---" << endl;
    cout << "思路：C++17 的 scoped_lock 能一次性锁多个 mutex，" << endl;
    cout << "      它内部用了巧妙的算法，保证不会死锁。" << endl;

    mutex m1, m2;

    thread ta([&]() {
        scoped_lock lock(m1, m2);  // 一次性全锁上
        this_thread::sleep_for(chrono::milliseconds(5));
        cout << "  哲学家A：scoped_lock 一把全拿到，开吃！" << endl;
    });

    thread tb([&]() {
        scoped_lock lock(m2, m1);  // 顺序反了也不怕！
        this_thread::sleep_for(chrono::milliseconds(5));
        cout << "  哲学家B：scoped_lock 一把全拿到，开吃！" << endl;
    });

    ta.join();
    tb.join();
    cout << "  都吃上了，scoped_lock 真香！" << endl;
}

// 方法3：尽量减少锁的持有时间
// 锁拿得越快还得越快，死锁的概率就越低。
// （这个是工程经验，不好用代码演示，道理讲清楚就行）

// 方法4：避免在持有锁时调用未知代码
// 比如回调函数，谁知道它里面会不会也加锁呢？
// （也是工程经验）

// 方法5：层次锁（lock hierarchy）
// 给锁分层，规定只能从高层往低层锁，不能反过来。
// 这样就不会形成循环等待。
class HierarchicalMutex {
private:
    mutex mtx;
    const int level;              // 当前锁的层级
    int prevLevel;                // 保存上一个层级
    static thread_local int currentLevel;  // 当前线程持有的最高层级

public:
    explicit HierarchicalMutex(int lvl) : level(lvl), prevLevel(0) {}

    void lock() {
        // 检查：只能锁层级比当前低的
        if (level >= currentLevel) {
            throw runtime_error("违反层次锁规则！不能从低层往高层锁");
        }
        mtx.lock();
        prevLevel = currentLevel;  // 保存之前的层级
        currentLevel = level;      // 更新当前层级
    }

    void unlock() {
        currentLevel = prevLevel;  // 恢复之前的层级
        mtx.unlock();
    }

    bool try_lock() {
        if (level >= currentLevel) return false;
        if (!mtx.try_lock()) return false;
        prevLevel = currentLevel;
        currentLevel = level;
        return true;
    }
};

// 每个线程有自己的 currentLevel（thread_local）
thread_local int HierarchicalMutex::currentLevel = INT_MAX;

void method5_hierarchical_lock() {
    cout << "\n--- 方法5：层次锁 ---" << endl;
    cout << "思路：给锁编上号（层级），规定只能从高往低锁，" << endl;
    cout << "      这样就不会形成循环等待的环。" << endl;
    cout << "      违反规则直接抛异常，在开发阶段就能发现问题。" << endl;

    HierarchicalMutex highLevel(100);  // 高层锁
    HierarchicalMutex midLevel(50);    // 中层锁
    HierarchicalMutex lowLevel(10);    // 低层锁

    // 正确用法：从高往低锁
    thread t1([&]() {
        try {
            highLevel.lock();
            cout << "  线程1：拿到高层锁" << endl;
            midLevel.lock();
            cout << "  线程1：拿到中层锁" << endl;
            lowLevel.lock();
            cout << "  线程1：拿到低层锁，全部成功！" << endl;
            lowLevel.unlock();
            midLevel.unlock();
            highLevel.unlock();
        } catch (const exception& e) {
            cout << "  线程1出错：" << e.what() << endl;
        }
    });

    // 错误用法：从低往高锁（会抛异常）
    thread t2([&]() {
        try {
            lowLevel.lock();
            cout << "  线程2：拿到低层锁" << endl;
            midLevel.lock();  // 这行会抛异常！因为低层不能往中层锁
            cout << "  线程2：这行应该不会执行" << endl;
            midLevel.unlock();
            lowLevel.unlock();
        } catch (const exception& e) {
            cout << "  线程2被拦住了：" << e.what() << endl;
            lowLevel.unlock();
        }
    });

    t1.join();
    t2.join();
    cout << "  层次锁在运行期帮你把关：违反顺序会抛异常，想死锁都难。" << endl;
}

// ============================================================
// 七、活锁（livelock）简介
// ============================================================
// 【老师傅的比喻】
// 两个人在走廊里迎面遇上：
//   A 往左让，B 也往左让——又撞上了
//   A 往右让，B 也往右让——又撞上了
//   就这样一直让来让去，谁也过不去
// 这就是活锁：线程没被阻塞（一直在运行），但就是没法往前推进。
// 跟死锁的区别：死锁是"都卡住不动"，活锁是"都在动但都白忙活"。
//
// 常见原因：多个线程都在"礼貌地"释放资源给对方，结果谁都拿不到完整的资源。
// 解决办法：给每个线程加个随机等待时间，或者按固定顺序来。

void demo_livelock() {
    cout << "\n========== 示例4：活锁风险（重试竞争） ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "活锁就是：线程都在跑，但谁也推进不了，一直在那儿'让来让去'。" << endl;
    cout << "比如两个人在走廊里遇上，你让我我让你，谁也过不去。" << endl;
    cout << endl;
    cout << "举个例子：两个线程，各拿一把锁，发现还需要另一把，" << endl;
    cout << "于是'礼貌地'先释放自己的，等一会儿再试。结果..." << endl;
    cout << "两个同时释放，同时再拿，又撞上了，无限循环。" << endl;
    cout << endl;
    cout << "下面是一个简化模拟：双方反复“拿一把、失败、放掉、重试”。" << endl;
    cout << "注意：这更准确地说是在演示“重试竞争/饥饿风险”，" << endl;
    cout << "      并不保证每次都构成严格意义上的活锁；活锁与调度时机密切相关。" << endl;
    cout << endl;

    timed_mutex m1, m2;
    atomic<int> attempts(0);
    atomic<int> successes(0);
    const int MAX_ATTEMPTS = 5;

    thread ta([&]() {
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {
            if (m1.try_lock_for(chrono::milliseconds(10))) {
                this_thread::sleep_for(chrono::milliseconds(5));
                if (m2.try_lock_for(chrono::milliseconds(10))) {
                    cout << "  线程A：终于两把都拿到了！" << endl;
                    successes++;
                    m2.unlock();
                    m1.unlock();
                    return;
                } else {
                    cout << "  线程A：拿不到 m2，先放了 m1，等会儿再试" << endl;
                    m1.unlock();
                    attempts++;
                    this_thread::sleep_for(chrono::milliseconds(15));
                }
            }
        }
        cout << "  线程A：试了 " << MAX_ATTEMPTS << " 次都没成，放弃了（反复失败）" << endl;
    });

    thread tb([&]() {
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {
            if (m2.try_lock_for(chrono::milliseconds(10))) {
                this_thread::sleep_for(chrono::milliseconds(5));
                if (m1.try_lock_for(chrono::milliseconds(10))) {
                    cout << "  线程B：终于两把都拿到了！" << endl;
                    successes++;
                    m1.unlock();
                    m2.unlock();
                    return;
                } else {
                    cout << "  线程B：拿不到 m1，先放了 m2，等会儿再试" << endl;
                    m2.unlock();
                    attempts++;
                    this_thread::sleep_for(chrono::milliseconds(15));
                }
            }
        }
        cout << "  线程B：试了 " << MAX_ATTEMPTS << " 次都没成，放弃了（反复失败）" << endl;
    });

    ta.join();
    tb.join();
    cout << "\n本轮成功拿到两把锁的线程数: " << successes.load() << " / 2" << endl;
    if (successes.load() < 2) {
        cout << "有线程反复“拿到一把、让掉、再试”，忙忙碌碌却无法完成操作。" << endl;
        cout << "这种反复失败接近活锁/饥饿：线程没被阻塞，但整体无法推进。" << endl;
        cout << "严格意义的活锁还要求线程在不断改变状态却始终无法达成目标。" << endl;
    } else {
        cout << "这次调度比较友好，两个线程都成功了。" << endl;
        cout << "活锁是调度相关的：多跑几次，可能就会看到双方不停重试的情况。" << endl;
    }
    cout << "解决办法：加随机退避（random backoff），别让双方总在同一节奏上重试。" << endl;
}

// ============================================================
// 八、优先级反转简介
// ============================================================
// 【老师傅的比喻】
// 公司里有三个级别：老板（高优先级）、经理（中优先级）、员工（低优先级）。
// 员工正在用会议室（持有锁），老板也要用会议室——
// 按理说老板应该等员工用完（锁是不能抢的）。
// 但这时候经理来了，把员工的活给打断了（经理优先级比员工高），
// 员工没法继续干活，会议室就一直占着，老板就一直等...
// 结果：最高优先级的老板，反而被中等优先级的经理给"坑"了。
// 这就是优先级反转：高优先级的线程被低优先级的线程阻塞，
// 而中间优先级的线程又抢了低优先级的 CPU 时间，导致高优先级更晚被执行。
//
// 解决方案：
//   1. 优先级继承：低优先级线程持有高优先级线程需要的锁时，
//      临时把它的优先级"提"上去，让它赶紧干完释放锁。
//   2. 优先级天花板：给锁设定一个最高优先级，持有锁的线程
//      直接升到这个优先级。
// （这俩一般是操作系统层面支持的，C++ 标准库没有直接封装，
//   知道有这么回事就行，面试可能会考）

void demo_priority_inversion() {
    cout << "\n========== 示例5：优先级反转简介 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "优先级反转是实时系统里的经典问题：" << endl;
    cout << "  高优先级线程 等 低优先级线程持有的锁" << endl;
    cout << "  中优先级线程 抢了 低优先级线程 的 CPU" << endl;
    cout << "  结果：高优先级反而更晚执行，像被'反转'了一样" << endl;
    cout << endl;
    cout << "C++ 标准库没有直接的优先级控制（那是操作系统的事），" << endl;
    cout << "但你得知道这个概念，尤其是做嵌入式、实时系统的。" << endl;
    cout << endl;
    cout << "解决办法主要有：" << endl;
    cout << "  1. 优先级继承：持有锁的低优先级线程临时升优先级" << endl;
    cout << "  2. 优先级天花板：锁有一个最高优先级，拿到就升到那个级别" << endl;
    cout << "  3. 最简单粗暴的：减少锁的使用，或者让持有锁的线程尽快释放" << endl;
}

// ============================================================
// 老师傅的话
// ============================================================
void laoShiFuDeHua() {
    cout << "\n========== 老师傅的话 ==========" << endl;
    cout << "徒弟啊，这节课讲的都是并发编程里的'坑'，给你掏心窝子说几句：" << endl;
    cout << endl;
    cout << "1. 并发 bug 最难调试" << endl;
    cout << "   竞态条件、死锁这些玩意儿，不是每次都出现。" << endl;
    cout << "   你本地跑一万次都好好的，到了用户那儿一跑就崩。" << endl;
    cout << "   所以啊，写代码的时候就要想清楚，别等出了 bug 再查。" << endl;
    cout << "   预防永远比调试容易。" << endl;
    cout << endl;
    cout << "2. 尽量不用锁" << endl;
    cout << "   锁是问题的根源。能用线程本地数据就别共享，" << endl;
    cout << "   能用消息队列通信就别共享内存，能用原子操作就别用锁。" << endl;
    cout << "   实在不行要用锁，也尽量把锁的粒度弄小一点。" << endl;
    cout << endl;
    cout << "3. 死锁需要四个必要条件同时成立" << endl;
    cout << "   互斥、持有并等待、不可剥夺、循环等待，破坏任意一个都能防死锁。" << endl;
    cout << "   按固定顺序加锁、层次锁主要破坏“循环等待”；" << endl;
    cout << "   std::scoped_lock 同时锁多个互斥量，则用死锁避免算法（尝试+回退）。" << endl;
    cout << endl;
    cout << "4. 多写多练，培养'并发直觉'" << endl;
    cout << "   看多了写多了，你一看到共享数据就会本能地问自己：" << endl;
    cout << "   '这个线程安全吗？会有竞态吗？会死锁吗？'" << endl;
    cout << "   有了这个直觉，你就入门了。" << endl;
}

// ============================================================
// 小练习
// ============================================================
void xiaoLianXi() {
    cout << "\n========== 小练习 ==========" << endl;
    cout << "纸上得来终觉浅，绝知此事要躬行：" << endl;
    cout << endl;
    cout << "1. 写一个有竞态条件的程序（比如两个线程同时 push_back 到 vector），" << endl;
    cout << "   然后用锁修复它。对比修复前后的结果差异。" << endl;
    cout << endl;
    cout << "2. 模拟哲学家就餐问题（5 个哲学家 5 根筷子）：" << endl;
    cout << "   - 先写出会死锁的版本" << endl;
    cout << "   - 再用'固定顺序加锁'的方法修复" << endl;
    cout << "   - 再试试用 scoped_lock 修复" << endl;
    cout << endl;
    cout << "3. 实现一个简单的线程安全的单例模式（Singleton）：" << endl;
    cout << "   - 思考：为什么双检锁（DCLP）在 C++11 之前有问题？" << endl;
    cout << "   - 提示：C++11 之后静态局部变量的初始化是线程安全的" << endl;
    cout << endl;
    cout << "4. 思考题：为什么说'无锁编程'比'有锁编程'更难？" << endl;
    cout << "   （提示：内存序、ABA 问题、指令重排...）" << endl;
    cout << endl;
    cout << "5. 思考题：volatile 关键字在 Java 和 C++ 里有什么区别？" << endl;
    cout << "   它能保证线程安全吗？" << endl;
    cout << endl;
    cout << "这些问题都好好琢磨琢磨，想明白了你就上了一个台阶！" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "================================================" << endl;
    cout << "  并发编程第三课：竞态条件与死锁（老师傅带徒弟）" << endl;
    cout << "================================================" << endl;

    demo_race_condition();          // 竞态条件演示
    demo_fix_race_condition();      // 修复竞态条件
    demo_deadlock();                // 死锁演示

    // 避免死锁的方法
    cout << "\n========== 如何避免死锁 ==========" << endl;
    method1_fixed_order();          // 方法1：固定顺序
    method2_scoped_lock();          // 方法2：scoped_lock
    cout << "\n--- 方法3：减少锁的持有时间 ---" << endl;
    cout << "（这是工程经验，记住：锁里只干必须干的事，快拿快放）" << endl;
    cout << "\n--- 方法4：避免持有锁时调用未知代码 ---" << endl;
    cout << "（回调函数、用户函数里可能也加锁，一不小心就死锁）" << endl;
    method5_hierarchical_lock();    // 方法5：层次锁

    demo_livelock();                // 活锁简介
    demo_priority_inversion();      // 优先级反转简介

    laoShiFuDeHua();                // 老师傅的话
    xiaoLianXi();                   // 小练习

    cout << "\n================================================" << endl;
    cout << "  三课全部完毕！恭喜你，并发编程入门了！" << endl;
    cout << "  但路还长，多写多练，熟能生巧。" << endl;
    cout << "================================================" << endl;

    return 0;
}
