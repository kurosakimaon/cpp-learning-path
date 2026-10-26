// ============================================================
// 04_atomic_memory.cpp - 原子操作与内存序入门
// 编译命令: g++ -std=c++17 -pthread 04_atomic_memory.cpp -o 04_atomic_memory
// ============================================================

#include <iostream>
#include <thread>
#include <atomic>
#include <bitset>
#include <vector>
#include <mutex>
#include <chrono>
#include <stack>
#include <memory>

using namespace std;

// ============================================================
// 一、什么是原子操作？
// ============================================================
// 老师傅比喻：
// 想象你和徒弟共用一个存钱罐（计数器）。每次你放一块钱进去，要经历三步：
//   1. 打开罐子看看里面有多少钱（读）
//   2. 在心里算一下加了一块钱后是多少（计算）
//   3. 把新的数量放回去（写）
// 如果你和徒弟同时去放钱，可能你刚读完，徒弟也读完了，
// 结果你们俩都以为原来有10块，都放回11块，最后变成11块而不是12块！
// 这就是"竞态条件"（race condition）。
//
// 原子操作就像是一个"一体式存钱罐"——放钱的动作是不可分割的，
// 要么完整做完，要么完全没做，中间不会被别人打断。
// ============================================================

// ---- 反面教材：普通 int 计数器（线程不安全） ----
void demo_race_condition() {
    cout << "\n=== 演示1：普通int vs 原子int（竞态条件） ===" << endl;

    // 用 volatile 只是防止编译器把整个循环优化成“加 100000”这一条指令，
    // 让竞态更容易被观察到。volatile 不是同步手段，也不提供原子性！
    volatile int normal_counter = 0; // 普通int，多线程同时写会出问题
    const int iterations = 100000;   // 每个线程累加次数
    const int num_threads = 10;      // 线程数

    vector<thread> threads;

    // 用普通 int 累加（可能丢数！）
    auto start = chrono::steady_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&normal_counter, iterations]() {
            for (int j = 0; j < iterations; ++j) {
                normal_counter++;  // 这一行不是原子的！底层是读-改-写三步
            }
        });
    }
    for (auto& t : threads) t.join();
    auto end = chrono::steady_clock::now();

    const int expected = num_threads * iterations;
    cout << "普通int结果: " << normal_counter
         << " (预期: " << expected << ")"
         << "，耗时: " << chrono::duration<double, milli>(end - start).count() << "ms" << endl;
    if (normal_counter != expected) {
        cout << "你看，结果少了！多个线程的读-改-写互相覆盖，这就是竞态条件。" << endl;
    } else {
        cout << "这次结果碰巧对了，但别高兴：无同步的并发读写是数据竞争（UB），" << endl;
        cout << "结果不可预测，换个编译器/优化级别/机器就可能出错。" << endl;
    }
}

// ---- 正确示范：atomic<int> 计数器（线程安全） ----
void demo_atomic_counter() {
    cout << "\n=== 演示2：atomic<int> 原子计数器 ===" << endl;

    atomic<int> atomic_counter{0};    // 原子int，多线程安全
    const int iterations = 100000;
    const int num_threads = 10;

    vector<thread> threads;

    auto start = chrono::steady_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&atomic_counter, iterations]() {
            for (int j = 0; j < iterations; ++j) {
                atomic_counter++;  // 原子操作，不可分割
                // 等价于 atomic_counter.fetch_add(1);
            }
        });
    }
    for (auto& t : threads) t.join();
    auto end = chrono::steady_clock::now();

    cout << "原子int结果: " << atomic_counter.load()
         << " (预期: " << num_threads * iterations << ")"
         << "，耗时: " << chrono::duration<double, milli>(end - start).count() << "ms" << endl;
    cout << "完美！结果正确，这就是原子操作的魔力。" << endl;
}

// ============================================================
// 二、原子操作的成员函数
// ============================================================
// 老师傅小课堂：
// std::atomic<T> 提供了一堆成员函数，各有各的用处：
//
//   load()          - 读取当前值（原子读）
//   store(val)      - 设置新值（原子写）
//   exchange(val)   - 设置新值并返回旧值（原子交换）
//   compare_exchange_strong(expected, desired)
//                   - CAS操作：如果当前值 == expected，就设为desired，返回true
//                     否则把expected更新为当前值，返回false
//   compare_exchange_weak(expected, desired)
//                   - 弱版本CAS，可能"虚假失败"（即使值相等也返回false），
//                     但性能更好，适合用在循环里
//   fetch_add(n)    - 加上n，返回旧值
//   fetch_sub(n)    - 减去n，返回旧值
//   fetch_and(x)    - 按位与，返回旧值
//   fetch_or(x)     - 按位或，返回旧值
//   fetch_xor(x)    - 按位异或，返回旧值
//
// 老师傅的比喻：
//   load 就像"看一眼存钱罐里有多少钱"
//   store 就像"直接把罐子里的钱换成指定数目"
//   exchange 就像"我把新钱放进去，同时把旧钱拿出来给我看看"
//   CAS 就像"我猜罐子里有10块，如果真是10块，就换成11块；不然告诉我现在有多少"
// ============================================================

void demo_atomic_operations() {
    cout << "\n=== 演示3：原子操作各种成员函数 ===" << endl;

    atomic<int> a{10};

    // load - 原子读取
    int val = a.load();
    cout << "load() 读取: " << val << endl;

    // store - 原子写入
    a.store(20);
    cout << "store(20) 后: " << a.load() << endl;

    // exchange - 原子交换，返回旧值
    int old_val = a.exchange(30);
    cout << "exchange(30)，旧值: " << old_val << "，新值: " << a.load() << endl;

    // fetch_add - 原子加，返回旧值
    old_val = a.fetch_add(5);
    cout << "fetch_add(5)，旧值: " << old_val << "，新值: " << a.load() << endl;

    // fetch_sub - 原子减
    old_val = a.fetch_sub(3);
    cout << "fetch_sub(3)，旧值: " << old_val << "，新值: " << a.load() << endl;

    // compare_exchange_strong - CAS操作
    int expected = 32;  // 我们期望当前值是32
    bool success = a.compare_exchange_strong(expected, 100);
    cout << "CAS(期望32, 设置100): " << (success ? "成功" : "失败")
         << "，当前值: " << a.load()
         << "，expected 现在为: " << expected
         << (success ? "（成功时保持不变）" : "（失败时被更新为当前值）") << endl;

    // 再试一次，这次期望的值是对的
    expected = 100;
    success = a.compare_exchange_strong(expected, 200);
    cout << "CAS(期望100, 设置200): " << (success ? "成功" : "失败")
         << "，当前值: " << a.load() << endl;

    // 按位操作
    atomic<int> bits{0b1010};  // 二进制 1010 = 10
    bits.fetch_and(0b1100);    // 按位与: 1010 & 1100 = 1000 = 8
    cout << "fetch_and(0b1100) 后: 0b" << bitset<4>(bits.load()).to_string() << endl;

    bits.fetch_or(0b0011);     // 按位或: 1000 | 0011 = 1011 = 11
    cout << "fetch_or(0b0011) 后: 0b" << bitset<4>(bits.load()).to_string() << endl;
}

// ============================================================
// 三、内存序（Memory Order）
// ============================================================
// 老师傅深入浅出：
// 内存序是并发编程里最烧脑的话题之一，咱慢慢来。
//
// 为什么需要内存序？
// 你写的代码顺序，CPU和编译器不一定按这个顺序执行！
// 为了提高性能，编译器会"指令重排"，CPU会"乱序执行"。
// 单线程下没问题，因为最终结果一样；但多线程下就可能出诡异的bug。
//
// 打个比方：你做饭，菜谱写的是"先洗菜，再切菜，再炒菜"。
// 单线程（你一个人）：你可以先切菜再洗菜吗？不行，因为切菜依赖洗菜。
// 但如果是两个独立的步骤，比如"洗菜"和"淘米"，谁先谁后都一样，
// 编译器/CPU可能会调换顺序来提高效率。
//
// 内存序就是告诉编译器和CPU："在多线程情况下，哪些操作的顺序不能乱！"
//
// C++有6种内存序，常用的有3种：
//   1. memory_order_seq_cst  - 顺序一致性（最强，默认）
//   2. memory_order_acquire / memory_order_release - 获取-释放语义
//   3. memory_order_relaxed  - 松散序（最弱，只保证原子性）
//
// 老师傅的经验之谈：
//   - 99%的情况用默认的 seq_cst 就够了，不要过早优化
//   - 真要优化，先从 acquire/release 开始
//   - relaxed 非常危险，只有在极其特殊的场景才用
// ============================================================

// ---- 示例：用 acquire/release 实现线程间同步 ----
// 场景：一个线程准备数据，另一个线程读取数据
// 我们要保证：数据准备好之前，读线程看不到数据
void demo_memory_order() {
    cout << "\n=== 演示4：内存序 - acquire/release 同步 ===" << endl;

    atomic<bool> ready{false};
    int data = 0;  // 普通变量，不是原子的

    // 写线程：准备数据，然后设置ready标志
    thread writer([&ready, &data]() {
        data = 42;  // 准备数据
        // release 语义：保证这之前的所有写操作，对 acquire 的线程可见
        // 就像说："我准备好了，前面的活都干完了，你可以读了"
        ready.store(true, memory_order_release);
        cout << "写线程：数据已准备好，data = " << data << endl;
    });

    // 读线程：等待ready标志，然后读取数据
    thread reader([&ready, &data]() {
        // acquire 语义：保证这之后的读操作，能看到 release 之前的所有写
        // 就像说："我确认你准备好了，那我开始读"
        while (!ready.load(memory_order_acquire)) {
            // 自旋等待
        }
        // 因为有 acquire/release 同步，这里一定能看到 data = 42
        cout << "读线程：数据已读取，data = " << data << endl;
    });

    writer.join();
    reader.join();

    cout << "acquire/release 保证了：写线程 release 之前的所有写，" << endl;
    cout << "都能被读线程 acquire 之后看到。这就是“同步”的含义。" << endl;
}

// ---- 示例：relaxed 只保证原子性，不保证顺序 ----
void demo_relaxed_order() {
    cout << "\n=== 演示5：relaxed 内存序（只保证原子性） ===" << endl;

    atomic<int> counter{0};

    // relaxed 只保证操作本身是原子的，不保证顺序
    // 就像大家往一个池子里扔球，每个扔的动作是完整的，
    // 但谁先扔谁后扔，没有保证
    vector<thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&counter, i]() {
            // relaxed 性能最好，但顺序没保证
            int old = counter.fetch_add(1, memory_order_relaxed);
            cout << "线程" << i << "：fetch_add 返回旧值 " << old << endl;
        });
    }
    for (auto& t : threads) t.join();

    cout << "最终 counter = " << counter.load(memory_order_relaxed) << endl;
    cout << "注意：虽然结果一定是5，但各个线程看到的旧值顺序是不确定的。" << endl;
    cout << "relaxed 适用于只关心最终结果、不关心执行顺序的场景。" << endl;
}

// ============================================================
// 四、实战：用 compare_exchange 实现自旋锁
// ============================================================
// 老师傅讲原理：
// 自旋锁（spinlock）是一种最简单的锁：
// 拿不到锁的时候，不睡觉，一直"自旋"（循环检查），直到拿到锁。
//
// 什么时候用自旋锁？
//   - 锁的持有时间很短（比如只是改个计数器）
//   - 线程切换的开销比自旋等待还大
//
// 用 CAS 实现自旋锁的思路：
//   - 用一个 atomic<bool> 表示锁的状态（false=空闲，true=已加锁）
//   - 加锁：用 CAS 尝试把 false 改成 true，成功就拿到锁了，
//     失败就继续循环（自旋）
//   - 解锁：把 true 改成 false
//
// 比喻：就像厕所门上的"有人/无人"牌子，
// 你看到无人（false），就翻成有人（true）进去；
// 如果看到有人（true），就在门口等着，时不时看一眼。
// ============================================================

class SpinLock {
private:
    atomic<bool> flag{false};  // false = 空闲, true = 已加锁

public:
    // 加锁
    void lock() {
        bool expected = false;
        // CAS 循环：尝试把 false 改成 true
        // 如果失败（说明别人拿着锁），expected 会被更新为 true，
        // 然后我们重置 expected 为 false，继续尝试
        while (!flag.compare_exchange_weak(expected, true,
                                           memory_order_acquire,
                                           memory_order_relaxed)) {
            expected = false;  // 重置期望值
            // 这里就是"自旋"——什么也不做，等下一次尝试
            // 实际工程中可以加个 pause 指令减少CPU功耗
        }
    }

    // 解锁
    void unlock() {
        // release 语义：保证解锁前的所有写都对其他线程可见
        flag.store(false, memory_order_release);
    }
};

void demo_spinlock() {
    cout << "\n=== 演示6：自旋锁（用 compare_exchange 实现） ===" << endl;

    SpinLock spinlock;
    int counter = 0;
    const int iterations = 10000;
    const int num_threads = 5;

    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&spinlock, &counter, iterations, i]() {
            for (int j = 0; j < iterations; ++j) {
                spinlock.lock();
                counter++;
                spinlock.unlock();
            }
            cout << "线程" << i << "完成累加" << endl;
        });
    }

    for (auto& t : threads) t.join();
    cout << "自旋锁保护的计数器结果: " << counter
         << " (预期: " << num_threads * iterations << ")" << endl;
    cout << "自旋锁搞定！注意：自旋锁适合锁持有时间很短的场景，" << endl;
    cout << "如果锁持有时间长，用 mutex 更合适（线程会睡眠，不占CPU）。" << endl;
}

// ============================================================
// 五、实战：无锁栈（Lock-Free Stack）
// ============================================================
// 老师傅讲无锁编程：
// 无锁（lock-free）编程就是不用互斥锁，只用原子操作来实现数据结构。
// 好处是：不会死锁、线程不会阻塞等待、性能可能更好。
// 坏处是：写起来非常难，非常容易出bug，而且bug极其难调试。
//
// 无锁栈的核心思路：
//   push：新节点的next指向当前栈顶，然后用CAS把栈顶改成新节点
//   pop：用CAS把栈顶改成栈顶的next，返回原来的栈顶
//
// 比喻：就像叠盘子，push就是把新盘子放最上面，
// 但你放的时候要确认你看到的那个"最上面的盘子"还在最上面，
// 不然别人可能已经放了新盘子了。
//
// 注意：本演示的节点只分配、不复用，所以这里不会出现 ABA。
// ABA 是"值从 A 变成 B 又变回 A"导致 CAS 误判；如果允许 delete 后
// 复用同一地址，就必须用标记指针（tagged pointer）或 Hazard Pointer 防护。
//
// 另外，本演示故意不释放弹出的节点。因为另一个线程可能已经读到
// 同一个 old_head，正准备访问 old_head->next；此时 delete 会造成
// use-after-free。真实的无锁栈必须配合 Hazard Pointer、epoch 回收
// 等安全内存回收方案，不能随手 delete。
// ============================================================

template <typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    atomic<Node*> head{nullptr};  // 栈顶指针，原子的

public:
    // 压栈
    void push(const T& value) {
        Node* new_node = new Node(value);
        // 把新节点的 next 指向当前栈顶
        new_node->next = head.load(memory_order_relaxed);

        // CAS 循环：尝试把栈顶改成新节点
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                           memory_order_release,
                                           memory_order_relaxed)) {
            // 如果失败，new_node->next 已经被更新为新的 head 了
            // 所以直接继续循环就行，不用手动重置
            // 这就是 compare_exchange 的巧妙之处！
        }
    }

    // 弹栈（返回指针，空栈返回 nullptr）
    bool pop(T& result) {
        // 必须用 acquire 读取栈顶：push 用 release 发布节点，
        // 只有 acquire 才能保证随后读取 old_head->next 是安全的。
        Node* old_head = head.load(memory_order_acquire);

        // CAS 循环：尝试把栈顶改成 old_head->next。
        // 失败时也会把 old_head 更新为新的 head；失败序同样用 acquire，
        // 保证下一轮读取新节点的 next 前已完成同步。
        while (old_head != nullptr &&
               !head.compare_exchange_weak(old_head, old_head->next,
                                           memory_order_acquire,
                                           memory_order_acquire)) {
            // 失败的话，old_head 已经被更新为新的 head 了
            // 继续尝试
        }

        if (old_head == nullptr) {
            return false;  // 栈空
        }

        result = old_head->data;

        // 注意：这里故意不 delete old_head！
        // 另一个线程可能刚读到同一个指针，正在用 old_head->next 做 CAS，
        // 这时 delete 会导致 use-after-free（即使本题的数据从不复用）。
        // 真实的无锁数据结构需要 Hazard Pointer / epoch 回收等方案来安全释放内存。
        // 本演示的代价是少量内存泄漏，仅用于说明 CAS 栈的结构。
        return true;
    }

    // 判断是否为空
    bool empty() const {
        return head.load(memory_order_relaxed) == nullptr;
    }
};

void demo_lock_free_stack() {
    cout << "\n=== 演示7：无锁栈（Lock-Free Stack） ===" << endl;

    LockFreeStack<int> stack;
    const int num_pushers = 3;
    const int num_popers = 2;
    const int items_per_pusher = 1000;

    vector<thread> threads;

    // 压栈线程
    atomic<int> total_pushed{0};
    for (int i = 0; i < num_pushers; ++i) {
        threads.emplace_back([&stack, &total_pushed, items_per_pusher, i]() {
            for (int j = 0; j < items_per_pusher; ++j) {
                stack.push(i * 10000 + j);  // 每个线程push自己编号的数
                total_pushed.fetch_add(1, memory_order_relaxed);
            }
            cout << "压栈线程" << i << "完成" << endl;
        });
    }

    // 弹栈线程
    atomic<int> total_popped{0};
    for (int i = 0; i < num_popers; ++i) {
        threads.emplace_back([&stack, &total_popped, i]() {
            int val;
            int count = 0;
            // 尝试弹很多次，能弹到就弹
            for (int j = 0; j < 100000; ++j) {
                if (stack.pop(val)) {
                    count++;
                }
            }
            total_popped.fetch_add(count, memory_order_relaxed);
            cout << "弹栈线程" << i << "弹出 " << count << " 个元素" << endl;
        });
    }

    for (auto& t : threads) t.join();

    cout << "总共压入: " << total_pushed.load() << " 个元素" << endl;
    cout << "总共弹出: " << total_popped.load() << " 个元素" << endl;
    cout << "(弹出数量可能小于压入数量，因为弹栈线程可能先跑完了)" << endl;

    // 把剩下的弹完
    int val;
    int remaining = 0;
    while (stack.pop(val)) {
        remaining++;
    }
    cout << "栈中剩余: " << remaining << " 个元素" << endl;
    cout << "验证: " << total_popped.load() + remaining
         << " == " << total_pushed.load() << " ? "
         << (total_popped.load() + remaining == total_pushed.load() ? "一致" : "不一致") << endl;

    cout << "\n无锁栈原理：push 和 pop 都用 CAS 操作栈顶指针，" << endl;
    cout << "栈操作本身没有加锁，多个线程可以同时操作。" << endl;
    cout << "但本演示为安全起见不释放弹出节点（会泄漏少量内存），" << endl;
    cout << "真实实现必须解决内存回收问题，否则会 use-after-free。" << endl;
    cout << "ABA问题、内存回收都是大坑，" << endl;
    cout << "实际项目中优先用成熟的库（比如 Intel TBB），别自己造轮子。" << endl;
}

// ============================================================
// 六、内存序完整对比总结
// ============================================================
// 老师傅画个表帮你理解：
//
// 内存序                    | 保证程度 | 适用场景
// --------------------------|---------|--------------------------
// memory_order_seq_cst      | 最强    | 默认值，99%情况用这个
// memory_order_acq_rel      | 较强    | 同时需要 acquire 和 release
// memory_order_acquire      | 读侧同步 | 读操作，配合 release 写
// memory_order_release      | 写侧同步 | 写操作，配合 acquire 读
// memory_order_consume      | 语义复杂 | 主流实现等同 acquire，不推荐使用
// memory_order_relaxed      | 最弱    | 只需要原子性，不需要顺序
//
// 注意：这张表只描述“约束强弱”，不是性能承诺。不同 CPU、
// 编译器和竞争程度下，各内存序的开销可能几乎没有差别。
//
// 老师傅的口诀：
//   - 拿不准就用 seq_cst，慢是慢点，但不会错
//   - 想优化先试试 acquire/release，大部分场景够用
//   - relaxed 别乱用，出了问题调试到怀疑人生
// ============================================================

void demo_seq_cst_vs_relaxed_perf() {
    cout << "\n=== 演示8：不同内存序的开销观察（仅供参考）===" << endl;

    const int iterations = 10000000;
    atomic<int> counter{0};

    // seq_cst（默认）
    auto start = chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        counter.fetch_add(1, memory_order_seq_cst);
    }
    auto end = chrono::steady_clock::now();
    auto seq_cst_time = chrono::duration<double, milli>(end - start).count();
    cout << "seq_cst:  " << seq_cst_time << "ms" << endl;

    counter.store(0);

    // relaxed
    start = chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        counter.fetch_add(1, memory_order_relaxed);
    }
    end = chrono::steady_clock::now();
    auto relaxed_time = chrono::duration<double, milli>(end - start).count();
    cout << "relaxed:  " << relaxed_time << "ms" << endl;

    if (relaxed_time > 0) {
        cout << "耗时比 seq_cst/relaxed = " << (seq_cst_time / relaxed_time) << endl;
    }
    cout << "单线程下两者接近是正常的：x86 上两种 fetch_add 往往都编译成" << endl;
    cout << "同一条 lock xadd，真正的差别在跨线程可见性与重排约束，" << endl;
    cout << "不能据此断言 relaxed 一定更快，也不能用它替代正确的同步。" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "============================================================" << endl;
    cout << "  并发编程进阶04：原子操作与内存序" << endl;
    cout << "============================================================" << endl;

    demo_race_condition();       // 演示竞态条件
    demo_atomic_counter();       // 原子计数器
    demo_atomic_operations();    // 各种原子操作
    demo_memory_order();         // acquire/release 内存序
    demo_relaxed_order();        // relaxed 内存序
    demo_spinlock();             // 自旋锁
    demo_lock_free_stack();      // 无锁栈
    demo_seq_cst_vs_relaxed_perf();  // 性能对比

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "徒弟啊，今天讲的原子操作和内存序，是并发编程的内功心法。" << endl;
    cout << "" << endl;
    cout << "1. 原子操作就是不可分割的操作，std::atomic 是C++给我们的武器。" << endl;
    cout << "   简单的计数器直接用 atomic<int>，比 mutex 轻量多了。" << endl;
    cout << "" << endl;
    cout << "2. CAS（compare_exchange）是无锁编程的基石，" << endl;
    cout << "   自旋锁、无锁栈、无锁队列，都是用CAS堆出来的。" << endl;
    cout << "   weak版本比strong版本快，但可能虚假失败，通常用在循环里。" << endl;
    cout << "" << endl;
    cout << "3. 内存序是高级中的高级话题，99%的情况用默认的 seq_cst 就够了。" << endl;
    cout << "   千万别上来就搞 relaxed，出了bug你根本调不出来。" << endl;
    cout << "   等你真正遇到性能瓶颈，再考虑优化内存序也不迟。" << endl;
    cout << "" << endl;
    cout << "4. 无锁编程听起来很酷，但水很深。" << endl;
    cout << "   ABA问题、内存回收、伪共享...坑一个比一个深。" << endl;
    cout << "   工作中真需要无锁数据结构，优先用成熟的库，别自己造。" << endl;
    cout << "" << endl;
    cout << "记住：先写对，再写快。过早优化是万恶之源。" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习" << endl;
    cout << "============================================================" << endl;
    cout << "1. 用 atomic<bool> 和 CAS 实现一个简单的“开关”，" << endl;
    cout << "   多个线程同时尝试打开开关，只有第一个能成功。" << endl;
    cout << "" << endl;
    cout << "2. 改造自旋锁，加入 backoff 策略：" << endl;
    cout << "   自旋几次后，如果还拿不到锁，就 sleep 一小会儿再试。" << endl;
    cout << "   （提示：用 std::this_thread::yield() 或 sleep_for）" << endl;
    cout << "" << endl;
    cout << "3. 思考一下：我们的无锁栈 pop 函数里如果直接 delete old_head，" << endl;
    cout << "   为什么会 use-after-free？（提示：别的线程可能还拿着这个指针）" << endl;
    cout << "   ABA 问题和这是两回事：查一下两者分别怎么解决（Hazard Pointer 等）。" << endl;
    cout << "" << endl;
    cout << "4. 用 atomic 实现一个简单的引用计数智能指针的雏形，" << endl;
    cout << "   支持多线程安全的引用计数增减。" << endl;

    return 0;
}
