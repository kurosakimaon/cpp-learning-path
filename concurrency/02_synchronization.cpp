// ============================================================
// 02_synchronization.cpp - 同步原语详解（老师傅带徒弟系列）
// 编译命令：g++ -std=c++17 02_synchronization.cpp -o demo2 -pthread
// ============================================================
//
// 【老师傅开场白】
// 徒弟啊，上节课咱们学了怎么开线程，今天咱们学一个更重要的东西——同步。
// 你想啊，一个车间里好几条流水线同时干活，要是大家都去抢同一台机器，
// 那不乱套了？同步原语就是给这些流水线"立规矩"的，谁先用、谁后等，
// 都得按规矩来。规矩立好了，大家各司其职；规矩没立好，轻则出错，重则死锁。

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <string>

using namespace std;

// ============================================================
// 一、为什么需要同步——资源竞争的例子
// ============================================================
// 【老师傅的比喻】
// 想象一个公共厕所（共享资源），门口没锁。两个人同时往里冲——
// 结果就是...嗯，画面太美不敢想。
// 互斥锁就是厕所门上的锁，一个人进去锁上，其他人就得在外面等。

// 这是一个"不安全"的计数器，多线程同时++会出问题
int unsafeCounter = 0;

// 这是一个"安全"的计数器，用 mutex 保护
int safeCounter = 0;
mutex counterMutex;

// 不安全的累加函数
void unsafeIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        // ++ 操作不是原子的！它分三步：读、加一、写回
        // 两个线程同时读同一个值，加完写回去，就少加了一次
        unsafeCounter++;
    }
}

// 安全的累加函数
void safeIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        counterMutex.lock();   // 加锁：别人别进来
        safeCounter++;         // 操作共享资源
        counterMutex.unlock(); // 解锁：下一个可以进来了
    }
}

// ============================================================
// 二、std::lock_guard —— RAII 锁，最简单最常用
// ============================================================
// 【老师傅的比喻】
// 你去澡堂子，进门拿钥匙锁柜门，走的时候开锁还钥匙。
// 但万一你走的时候忘了还钥匙？lock_guard 就像一个自动还钥匙的管家——
// 你进去的时候它帮你锁上，你走的时候（不管正常走还是摔一跤出去）
// 它都自动帮你开锁还钥匙。

mutex printMutex;  // 保护 cout 的互斥锁

void safePrint(const string& msg) {
    // lock_guard 构造时加锁，析构时自动解锁
    // 即使中间抛异常，也能保证解锁（异常安全）
    lock_guard<mutex> guard(printMutex);
    cout << msg << endl;
}

// ============================================================
// 三、std::unique_lock —— 功能更全的锁
// ============================================================
// 【老师傅的比喻】
// lock_guard 是个老实人，只会"进门锁、出门开"。
// unique_lock 是个灵活的伙计，可以：
//   - 先不锁，等会儿再锁（延迟加锁 defer_lock）
//   - 中间可以临时解锁，用完再锁上
//   - 可以把锁的所有权转移给别人
//   - 可以跟条件变量配合使用
// 代价就是：比 lock_guard 稍微重那么一丢丢（性能差别不大）

// 示例：unique_lock 的延迟加锁和手动解锁
void demo_unique_lock() {
    mutex mtx;

    // defer_lock：先不锁，我自己决定什么时候锁
    unique_lock<mutex> lock(mtx, defer_lock);

    safePrint("  unique_lock 创建了，但还没加锁");

    lock.lock();  // 现在才加锁
    safePrint("  现在加锁了，操作共享资源...");

    lock.unlock();  // 临时解锁，干点不涉及共享资源的事
    safePrint("  临时解锁，干点别的...");

    lock.lock();  // 再锁上
    safePrint("  又锁上了，继续操作...");

    // 出作用域时，如果还锁着，会自动解锁
}

// ============================================================
// 四、std::lock —— 一次性锁多个 mutex（防止死锁）
// ============================================================
// 【老师傅的比喻】
// 两个人吃饭，需要一双筷子（两个锁）。
// 如果 A 拿了左边的等右边的，B 拿了右边的等左边的——死锁！
// std::lock 的思路是：要么两把筷子都拿到，要么一把都不拿，等着。
// 这样就不会出现"拿一个等一个"的情况。

mutex mtxA, mtxB;

void demo_std_lock() {
    // std::lock 可以同时锁多个 mutex，它内部会用巧妙的算法避免死锁
    // 但注意：锁了之后要手动解锁，或者用 lock_guard/unique_lock 接管

    // 方法：先用 defer_lock 创建 unique_lock（不锁），再用 std::lock 一起锁
    unique_lock<mutex> lockA(mtxA, defer_lock);
    unique_lock<mutex> lockB(mtxB, defer_lock);

    std::lock(lockA, lockB);  // 一次性把两个都锁上

    safePrint("  mtxA 和 mtxB 都被 std::lock 同时锁住了，安全！");

    // 出作用域时自动解锁
}

// ============================================================
// 五、std::scoped_lock —— C++17 新玩具，lock 的 RAII 版本
// ============================================================
// 【老师傅的话】
// 这是 C++17 新加的，是 std::lock 的 RAII 版本。
// 说白了就是：你给它几个 mutex，它一次性都给你锁上，
// 出作用域自动都解开。用起来比 std::lock + unique_lock 方便多了。
// 新项目推荐用这个！

void demo_scoped_lock() {
    mutex m1, m2, m3;

    // scoped_lock 可以锁任意数量的 mutex
    scoped_lock sl(m1, m2, m3);

    safePrint("  scoped_lock 一把锁了三个 mutex，真香！");
    // 出作用域自动全解锁
}

// ============================================================
// 六、std::recursive_mutex —— 递归锁
// ============================================================
// 【老师傅的比喻】
// 普通的 mutex 就像一个只能穿一件衣服的人，再穿一件就卡住了。
// 递归锁呢，就是能穿好几件衣服，穿几件脱几件，全脱了才算解锁。
// 什么时候用？比如一个函数内部调用了另一个也需要同一把锁的函数。
// 但老师傅劝你：能不用就不用，往往说明代码设计有问题。

recursive_mutex recMtx;

void recursiveFunc(int depth) {
    if (depth <= 0) return;

    recMtx.lock();  // 同一线程可以重复加锁
    safePrint("  递归深度 " + to_string(depth) + "：加锁了");

    recursiveFunc(depth - 1);  // 递归调用，里面又会加锁

    safePrint("  递归深度 " + to_string(depth) + "：解锁了");
    recMtx.unlock();  // 加了几次就要解几次
}

// ============================================================
// 七、std::timed_mutex —— 超时锁
// ============================================================
// 【老师傅的比喻】
// 普通锁是死等，等到天荒地老也要等到。
// 超时锁就是：等你一会儿，等不到我就走了，干点别的去。
// 适合那些"拿不到锁也不能一直傻等"的场景。

timed_mutex timedMtx;

void demo_timed_mutex() {
    // try_lock_for：等一段时间，拿到就返回 true，超时返回 false
    if (timedMtx.try_lock_for(chrono::milliseconds(100))) {
        safePrint("  timed_mutex：等了一会儿，拿到锁了！");
        timedMtx.unlock();
    } else {
        safePrint("  timed_mutex：等超时了，没拿到锁");
    }

    // try_lock_until：等到某个时间点
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(50);
    if (timedMtx.try_lock_until(deadline)) {
        safePrint("  timed_mutex：在截止时间前拿到锁了！");
        timedMtx.unlock();
    }
}

// ============================================================
// 八、std::shared_mutex —— 读写锁（C++17）
// ============================================================
// 【老师傅的比喻】
// 想象一个图书阅览室：
//   - 大家可以一起看书（读操作可以并发）
//   - 但有人要写书（写操作）的时候，其他人都得出去
//   - 反过来，有人在看书，写书的人也得等大家都看完
// 这就是读写锁：读读不互斥，读写互斥，写写互斥。
// 适合读多写少的场景，性能比普通 mutex 好。

shared_mutex shMtx;
string sharedData = "初始内容";  // 共享数据

// 读操作：用 shared_lock（共享锁）
void reader(int id) {
    shared_lock<shared_mutex> lock(shMtx);  // 共享锁，多个读者可以同时持有
    safePrint("  读者" + to_string(id) + " 读到：" + sharedData);
    // 读完自动释放共享锁
}

// 写操作：用 unique_lock 或 lock_guard（独占锁）
void writer(int id, const string& newContent) {
    unique_lock<shared_mutex> lock(shMtx);  // 独占锁，写的时候别人都不能进来
    safePrint("  写者" + to_string(id) + " 修改内容为：" + newContent);
    sharedData = newContent;
    this_thread::sleep_for(chrono::milliseconds(20));  // 假装写得慢
    // 写完自动释放独占锁
}

// ============================================================
// 九、std::condition_variable —— 条件变量
// ============================================================
// 【老师傅的比喻】
// 生产者-消费者模型，就像食堂打饭：
//   - 厨师（生产者）做好饭放到窗口
//   - 学生（消费者）排队等饭
//   - 没饭的时候，学生就在那儿等着（睡觉），不瞎转悠
//   - 饭做好了，厨师喊一声"饭好了"，学生们就醒过来抢饭
// 条件变量就是干这个的：让线程在某个条件不满足时睡觉，
// 条件满足了被叫醒，避免轮询浪费 CPU。

// 生产者-消费者队列
queue<int> productQueue;
mutex queueMutex;
condition_variable cv;
bool done = false;  // 生产完成标志

// 生产者
void producer(int id, int count) {
    for (int i = 1; i <= count; ++i) {
        this_thread::sleep_for(chrono::milliseconds(15));  // 生产需要时间

        {
            lock_guard<mutex> lock(queueMutex);
            productQueue.push(id * 100 + i);  // 产品编号：生产者号 + 序号
            safePrint("  生产者" + to_string(id) + " 生产了产品 " + to_string(id * 100 + i));
        }
        cv.notify_one();  // 通知一个消费者：有货了！
    }

    // 生产完毕，设置标志并通知所有消费者
    {
        lock_guard<mutex> lock(queueMutex);
        done = true;
    }
    cv.notify_all();  // 都醒醒，没货了，散了吧
}

// 消费者
void consumer(int id) {
    while (true) {
        unique_lock<mutex> lock(queueMutex);

        // wait 的用法：等条件满足
        // 注意：wait 会先解锁，然后睡觉；被叫醒后会重新加锁，再检查条件
        // 为什么要循环检查？因为可能有"虚假唤醒"（spurious wakeup）
        cv.wait(lock, []() {
            return !productQueue.empty() || done;  // 队列非空 或 生产结束
        });

        if (!productQueue.empty()) {
            int product = productQueue.front();
            productQueue.pop();
            lock.unlock();  // 拿到东西就解锁，别占着茅坑不拉屎

            safePrint("  消费者" + to_string(id) + " 消费了产品 " + to_string(product));
        } else if (done) {
            // 生产结束了，队列也空了，走人
            safePrint("  消费者" + to_string(id) + "：没货了，下班！");
            break;
        }
    }
}

// ============================================================
// 各个示例函数
// ============================================================

// 示例1：为什么需要同步（不安全 vs 安全的计数器）
void demo1_why_synchronize() {
    cout << "\n========== 示例1：为什么需要同步 ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "两个线程同时对一个计数器++，各加 100000 次。" << endl;
    cout << "理论上结果应该是 200000，但不加锁的话...你自己看吧。" << endl;
    cout << "（因为 ++ 不是原子操作，中间可能被打断，导致丢数据）" << endl;
    cout << endl;

    const int TIMES = 100000;

    // 不安全的版本
    unsafeCounter = 0;
    thread t1(unsafeIncrement, TIMES);
    thread t2(unsafeIncrement, TIMES);
    t1.join();
    t2.join();
    cout << "不加锁的结果：" << unsafeCounter << "（应该是 " << 2 * TIMES << "，错了吧？）" << endl;

    // 安全的版本
    safeCounter = 0;
    thread t3(safeIncrement, TIMES);
    thread t4(safeIncrement, TIMES);
    t3.join();
    t4.join();
    cout << "加锁的结果：" << safeCounter << "（正确！）" << endl;
}

// 示例2：lock_guard 的使用
void demo2_lock_guard() {
    cout << "\n========== 示例2：std::lock_guard ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "lock_guard 是最简单的 RAII 锁：构造时加锁，析构时解锁。" << endl;
    cout << "好处是不用担心忘记 unlock，也不用担心异常时锁没释放。" << endl;
    cout << "大部分场景用它就够了。" << endl;
    cout << endl;

    vector<thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            safePrint("线程 " + to_string(i) + " 在用 lock_guard 安全打印");
        });
    }
    for (auto& t : threads) t.join();

    cout << "\n你看，输出整整齐齐的，不会像不加锁那样乱成一锅粥。" << endl;
}

// 示例3：unique_lock 的灵活用法
void demo3_unique_lock() {
    cout << "\n========== 示例3：std::unique_lock ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "unique_lock 比 lock_guard 功能多：可延迟加锁、可手动解锁、可移动。" << endl;
    cout << "代价是一点点性能开销（一般感知不到）。" << endl;
    cout << "需要灵活控制锁的时候用它，比如跟条件变量配合。" << endl;
    cout << endl;

    demo_unique_lock();
}

// 示例4：std::lock 和 scoped_lock
void demo4_lock_and_scoped_lock() {
    cout << "\n========== 示例4：std::lock 与 scoped_lock ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "需要同时锁多个 mutex 的时候，千万别一个一个锁，容易死锁。" << endl;
    cout << "用 std::lock 或者 C++17 的 scoped_lock，一次性全锁上。" << endl;
    cout << "scoped_lock 是 RAII 版本的 std::lock，用起来更省心。" << endl;
    cout << endl;

    demo_std_lock();
    demo_scoped_lock();
}

// 示例5：递归锁
void demo5_recursive_mutex() {
    cout << "\n========== 示例5：std::recursive_mutex ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "递归锁允许同一个线程多次加锁，加几次解几次。" << endl;
    cout << "什么时候用？比如递归函数里需要加锁的场景。" << endl;
    cout << "但老师傅提醒你：能用普通锁重构就别用递归锁，" << endl;
    cout << "递归锁往往意味着代码结构可以优化。" << endl;
    cout << endl;

    recursiveFunc(3);
}

// 示例6：超时锁
void demo6_timed_mutex() {
    cout << "\n========== 示例6：std::timed_mutex ==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "timed_mutex 支持 try_lock_for 和 try_lock_until，" << endl;
    cout << "拿不到锁就等一会儿，等不到就放弃，不会死等。" << endl;
    cout << "适合那些'不能因为拿不到锁就一直卡住'的场景。" << endl;
    cout << endl;

    demo_timed_mutex();
}

// 示例7：读写锁 shared_mutex
void demo7_shared_mutex() {
    cout << "\n========== 示例7：std::shared_mutex（读写锁）==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "读写锁的规则：读读共享，读写互斥，写写互斥。" << endl;
    cout << "读多用 shared_lock（共享模式），写用 unique_lock（独占模式）。" << endl;
    cout << "读多写少的场景用它，性能比普通 mutex 好不少。" << endl;
    cout << endl;

    vector<thread> threads;

    // 先开 3 个读者
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(reader, i);
    }

    // 再来一个写者
    threads.emplace_back(writer, 1, "第一次修改");

    // 再来 2 个读者
    for (int i = 4; i <= 5; ++i) {
        threads.emplace_back(reader, i);
    }

    // 再来一个写者
    threads.emplace_back(writer, 2, "第二次修改");

    // 最后 2 个读者
    for (int i = 6; i <= 7; ++i) {
        threads.emplace_back(reader, i);
    }

    for (auto& t : threads) t.join();

    cout << "\n观察输出你会发现：读者可以一起读，写者必须独占。" << endl;
}

// 示例8：条件变量（生产者消费者模型）
void demo8_condition_variable() {
    cout << "\n========== 示例8：condition_variable（生产者消费者）==========" << endl;
    cout << "【老师傅讲解】" << endl;
    cout << "条件变量让线程在条件不满足时睡觉（不占 CPU），条件满足时被叫醒。" << endl;
    cout << "经典应用就是生产者-消费者模型。" << endl;
    cout << "注意：wait 一定要循环检查条件，防止虚假唤醒。" << endl;
    cout << "      还有，notify_one 只叫醒一个，notify_all 叫醒所有。" << endl;
    cout << endl;

    done = false;  // 重置标志

    thread prod(producer, 1, 8);    // 1个生产者，生产8个产品
    thread cons1(consumer, 1);      // 消费者1
    thread cons2(consumer, 2);      // 消费者2

    prod.join();
    cons1.join();
    cons2.join();
}

// ============================================================
// 各种锁的对比总结
// ============================================================
void lock_comparison() {
    cout << "\n========== 各种锁的使用场景对比 ==========" << endl;
    cout << "锁的种类             | 特点                      | 适用场景" << endl;
    cout << "---------------------|---------------------------|--------------------------" << endl;
    cout << "std::mutex           | 最基础的互斥锁            | 一般场景，首选" << endl;
    cout << "std::lock_guard      | RAII，简单                | 简单加锁解锁，推荐！" << endl;
    cout << "std::unique_lock     | 功能全，灵活              | 条件变量、需要手动控制" << endl;
    cout << "std::scoped_lock     | C++17，可锁多个           | 同时锁多个，推荐！" << endl;
    cout << "std::recursive_mutex | 同线程可重复加锁          | 递归等特殊场景（少用）" << endl;
    cout << "std::timed_mutex     | 支持超时                  | 不能死等的场景" << endl;
    cout << "std::shared_mutex    | 读写锁，读读共享          | 读多写少，C++17" << endl;
    cout << endl;
    cout << "【选择建议】" << endl;
    cout << "  1. 简单场景直接 lock_guard<mutex>，够用了。" << endl;
    cout << "  2. 要跟条件变量配合，用 unique_lock。" << endl;
    cout << "  3. 要同时锁多个，用 scoped_lock（C++17）。" << endl;
    cout << "  4. 读多写少，用 shared_mutex。" << endl;
    cout << "  5. 递归锁能不用就不用，先想想能不能改代码结构。" << endl;
}

// ============================================================
// 老师傅的话
// ============================================================
void laoShiFuDeHua() {
    cout << "\n========== 老师傅的话 ==========" << endl;
    cout << "徒弟啊，今天讲的锁有点多，给你总结几句心里话：" << endl;
    cout << endl;
    cout << "1. 锁的粒度要适中" << endl;
    cout << "   锁太大（一进函数就锁），并发度就低，跟串行差不多。" << endl;
    cout << "   锁太小（只锁一行代码），容易漏，而且频繁加解锁也有开销。" << endl;
    cout << "   找到那个'刚刚好'的粒度，是个手艺活。" << endl;
    cout << endl;
    cout << "2. 能不加锁就不加锁" << endl;
    cout << "   加锁是没办法的办法。能用线程本地数据就别共享，" << endl;
    cout << "   能用无锁数据结构就用无锁（但无锁水很深，初学者慎用），" << endl;
    cout << "   能通过消息传递通信就别共享内存。" << endl;
    cout << endl;
    cout << "3. 尽量减少锁的持有时间" << endl;
    cout << "   锁里面只干必须干的事，打印、sleep、耗时计算都放锁外面。" << endl;
    cout << "   占着茅坑不拉屎，是并发编程的大忌。" << endl;
    cout << endl;
    cout << "4. 别在持有锁的时候调用未知代码" << endl;
    cout << "   比如回调函数、用户传入的函数。谁知道它里面会不会也加锁？" << endl;
    cout << "   一不小心就死锁了。" << endl;
}

// ============================================================
// 小练习
// ============================================================
void xiaoLianXi() {
    cout << "\n========== 小练习 ==========" << endl;
    cout << "光说不练假把式，来做几道题：" << endl;
    cout << endl;
    cout << "1. 写一个线程安全的栈（Stack），支持 push、pop、top、empty。" << endl;
    cout << "   用 mutex + lock_guard 保护。" << endl;
    cout << endl;
    cout << "2. 用 condition_variable 实现一个线程安全的阻塞队列：" << endl;
    cout << "   - push 时如果满了就等，pop 时如果空了就等" << endl;
    cout << "   - 支持设定最大容量" << endl;
    cout << endl;
    cout << "3. 用 shared_mutex 实现一个简单的缓存（Cache）：" << endl;
    cout << "   - 读操作（get）用 shared_lock" << endl;
    cout << "   - 写操作（put/remove）用 unique_lock" << endl;
    cout << endl;
    cout << "4. 思考题：为什么条件变量的 wait 一定要配合 mutex？" << endl;
    cout << "   （提示：想想条件检查和 wait 之间的竞态）" << endl;
    cout << endl;
    cout << "5. 思考题：虚假唤醒（spurious wakeup）是什么？为什么会有？" << endl;
    cout << endl;
    cout << "动手写一写，比看十遍书都管用！" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "================================================" << endl;
    cout << "  并发编程第二课：同步原语详解（老师傅带徒弟）" << endl;
    cout << "================================================" << endl;

    demo1_why_synchronize();    // 为什么需要同步
    demo2_lock_guard();         // lock_guard
    demo3_unique_lock();        // unique_lock
    demo4_lock_and_scoped_lock(); // std::lock 和 scoped_lock
    demo5_recursive_mutex();    // 递归锁
    demo6_timed_mutex();        // 超时锁
    demo7_shared_mutex();       // 读写锁
    demo8_condition_variable(); // 条件变量

    lock_comparison();          // 锁的对比总结
    laoShiFuDeHua();            // 老师傅的话
    xiaoLianXi();               // 小练习

    cout << "\n================================================" << endl;
    cout << "  第二课完毕，下节课咱们讲竞态条件与死锁！" << endl;
    cout << "================================================" << endl;

    return 0;
}
