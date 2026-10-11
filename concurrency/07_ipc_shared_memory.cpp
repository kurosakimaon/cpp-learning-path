// ============================================================
// 07_ipc_shared_memory.cpp - 进程间通信（Boost.Interprocess）
// 编译命令: g++ -std=c++17 07_ipc_shared_memory.cpp -lboost_system -lrt -pthread -o 07_ipc_shared_memory
//
// 注意：本文件使用 Boost.Interprocess 库
// Windows 下需要安装 Boost 并正确配置头文件和库路径
// ============================================================

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <stdexcept>

// Boost.Interprocess 头文件
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

using namespace std;
namespace bip = boost::interprocess;

// ============================================================
// 一、什么是进程间通信（IPC）
// ============================================================
// 老师傅讲故事：
// 前面咱们讲的多线程，都是在同一个进程里的"一家人"，
// 共享同一个地址空间，沟通起来很方便——用个全局变量就行。
//
// 但有时候，你需要和"外人"打交道——另一个进程。
// 比如：
//   - 你的程序要和数据库通信
//   - 两个不同的程序要交换数据
//   - 微服务架构下，服务之间要调用
//
// 进程之间地址空间是隔离的（这是操作系统的保护机制），
// A进程不能直接读B进程的内存。所以需要专门的"沟通渠道"，
// 这就是进程间通信（Inter-Process Communication，简称IPC）。
//
// 比喻：
// 多线程就像一家人住在一起，说话直接喊就行。
// 多进程就像两户邻居，要说话得打电话、写信、或者串门。
// ============================================================

// ============================================================
// 二、常见IPC方式对比
// ============================================================
// 老师傅画个表：
//
// IPC方式       | 速度 | 复杂度 | 适用场景
// --------------|------|-------|--------------------------
// 管道(Pipe)    | 中   | 低    | 父子进程、单向数据流
// 命名管道(FIFO)| 中   | 低    | 无亲缘关系进程、单向
// 消息队列      | 中   | 中    | 结构化消息、优先级
// 共享内存      | 最快 | 高    | 大量数据、需要同步
// 信号量        | -    | 中    | 进程间同步（不是传数据）
// 套接字(Socket)| 最慢 | 中    | 跨机器、最通用
//
// 老师傅的经验之谈：
//   - 同一台机器上大量数据传输：共享内存 + 信号量/互斥锁
//   - 小消息、结构化数据：消息队列
//   - 跨机器通信：socket（TCP/UDP）
//   - 简单的命令传递：管道
// ============================================================

// ============================================================
// 三、Boost.Interprocess 库介绍
// ============================================================
// 老师傅说：
// C++标准库里没有IPC的东西（标准库只管单进程内的事），
// 所以咱们得用第三方库。Boost.Interprocess 是最常用的一个。
//
// 它提供了什么？
//   - 共享内存（shared memory）
//   - 内存映射文件（memory-mapped file）
//   - 进程间同步原语（mutex, condition, semaphore...）
//   - 消息队列（message queue）
//   - 命名管道（named pipe）
//   - 还支持在共享内存里构造STL容器（vector, string, map...）
//
// 很强大对吧？咱们一个个来学。
// ============================================================

// ============================================================
// 四、共享内存（Shared Memory）
// ============================================================
// 老师傅讲原理：
// 共享内存是最快的IPC方式，为什么？
// 因为它就是让两个进程映射同一块物理内存到各自的地址空间，
// 一个进程写了，另一个进程立刻就能看到——不需要拷贝数据！
//
// 比喻：
// 就像两家共用一面墙，墙上贴个字条，两家人都能看到。
// 不用打电话、不用送信，直接看墙就行——最快！
//
// 但问题也来了：
// 两个人同时往墙上写字怎么办？写乱了怎么办？
// 这就需要同步机制（互斥锁、条件变量等）。
//
// 所以说：共享内存最快，但同步最麻烦。
// ============================================================

// ---- 演示1：最基础的共享内存 ----
// 注意：实际使用中应该是两个不同的进程
// 这里我们在同一个进程里模拟，方便你看到效果
void demo_basic_shared_memory() {
    cout << "\n=== 演示1：基础共享内存 ===" << endl;

    // 共享内存的名字（系统范围内唯一，就像文件名一样）
    const char* shm_name = "my_demo_shm";

    // ---- 第一步：创建/打开共享内存 ----
    // 相当于：我要在系统里开一块共享内存，名字叫 shm_name
    // open_or_create：有就打开，没有就创建
    // read_write：读写模式
    try {
        // 先删除可能存在的旧共享内存（防止上次运行没清理）
        bip::shared_memory_object::remove(shm_name);

        // 创建共享内存对象
        bip::shared_memory_object shm(
            bip::open_or_create,  // 打开或创建
            shm_name,             // 名字
            bip::read_write       // 读写模式
        );

        // 设置大小（刚创建的共享内存大小是0，得手动设）
        shm.truncate(1024);  // 1024字节

        cout << "共享内存创建成功，名字: " << shm_name << endl;

        // ---- 第二步：映射到当前进程的地址空间 ----
        // 共享内存是内核对象，得映射到进程的地址空间才能用
        // 就像你租了个仓库，得有条路能走进去才行
        bip::mapped_region region(
            shm,              // 共享内存对象
            bip::read_write   // 映射方式
        );

        cout << "映射成功，地址: " << region.get_address()
             << "，大小: " << region.get_size() << " 字节" << endl;

        // ---- 第三步：写入数据 ----
        char* data = static_cast<char*>(region.get_address());
        const char* message = "Hello, 共享内存！";
        strcpy(data, message);  // 直接往共享内存里写
        cout << "写入数据: " << message << endl;

        // ---- 第四步：读取数据（模拟另一个进程读取） ----
        // 实际使用中，这应该是另一个进程干的事
        // 另一个进程也是同样的步骤：打开 -> 映射 -> 读取
        {
            bip::shared_memory_object shm2(
                bip::open_only,      // 只打开（不创建）
                shm_name,
                bip::read_only       // 只读模式
            );

            bip::mapped_region region2(shm2, bip::read_only);
            const char* data2 = static_cast<const char*>(region2.get_address());
            cout << "另一个进程读取到: " << data2 << endl;
        }

        // ---- 第五步：清理 ----
        // 注意：共享内存在内核中，进程退出了它还在！
        // 必须手动删除，不然一直占着内存
        bip::shared_memory_object::remove(shm_name);
        cout << "共享内存已删除" << endl;

    } catch (const bip::interprocess_exception& e) {
        cerr << "共享内存出错: " << e.what() << endl;
        // 出错了也清理一下
        bip::shared_memory_object::remove(shm_name);
    }
}

// ============================================================
// 五、带同步的共享内存（interprocess_mutex）
// ============================================================
// 老师傅说：
// 上面那个例子太简单了，两个进程同时写怎么办？
// 就像两个人同时往白板上写字，肯定乱套。
//
// 所以共享内存通常要配合同步机制一起使用。
// Boost.Interprocess 提供了进程间的同步原语：
//   - interprocess_mutex     进程间互斥锁
//   - interprocess_condition 进程间条件变量
//   - interprocess_semaphore 进程间信号量
//   - named_mutex            命名互斥锁
//
// 注意：这些同步对象也要放在共享内存里！
// 不然两个进程拿到的不是同一个锁，有啥用？
// ============================================================

// 共享内存里的数据结构
// 注意：这个结构体里的所有东西都要放在共享内存里
struct SharedData {
    bip::interprocess_mutex mutex;    // 进程间互斥锁
    bip::interprocess_condition cond; // 进程间条件变量
    int counter;                      // 共享计数器
    bool ready;                       // 数据就绪标志
    char message[256];                // 消息缓冲区

    SharedData() : counter(0), ready(false) {
        message[0] = '\0';
    }
};

void demo_synchronized_shm() {
    cout << "\n=== 演示2：带同步的共享内存 ===" << endl;

    const char* shm_name = "sync_shm_demo";

    try {
        // 清理旧的
        bip::shared_memory_object::remove(shm_name);

        // 创建共享内存
        bip::shared_memory_object shm(
            bip::open_or_create,
            shm_name,
            bip::read_write
        );
        shm.truncate(sizeof(SharedData));

        // 映射
        bip::mapped_region region(shm, bip::read_write);
        SharedData* data = new (region.get_address()) SharedData();  // placement new

        cout << "共享数据结构已创建" << endl;

        // ---- 模拟两个进程交替累加计数器 ----
        // 实际中这应该是两个进程，这里用两个线程模拟
        // （因为 interprocess_mutex 也能在线程间用，虽然没必要）

        auto worker1 = [data]() {
            for (int i = 0; i < 5; ++i) {
                bip::scoped_lock<bip::interprocess_mutex> lock(data->mutex);
                data->counter++;
                cout << "  进程A：counter = " << data->counter << endl;
                lock.unlock();
                data->cond.notify_one();

                this_thread::sleep_for(chrono::milliseconds(100));
            }
        };

        auto worker2 = [data]() {
            for (int i = 0; i < 5; ++i) {
                bip::scoped_lock<bip::interprocess_mutex> lock(data->mutex);
                data->counter++;
                cout << "  进程B：counter = " << data->counter << endl;
                lock.unlock();
                data->cond.notify_one();

                this_thread::sleep_for(chrono::milliseconds(100));
            }
        };

        thread t1(worker1);
        thread t2(worker2);
        t1.join();
        t2.join();

        cout << "最终 counter = " << data->counter << " (预期 10)" << endl;
        cout << "interprocess_mutex 保证了多进程下的数据安全。" << endl;

        // 析构（placement new 的对象要手动调用析构函数）
        data->~SharedData();

        // 清理
        bip::shared_memory_object::remove(shm_name);

    } catch (const bip::interprocess_exception& e) {
        cerr << "出错: " << e.what() << endl;
        bip::shared_memory_object::remove(shm_name);
    }
}

// ============================================================
// 六、managed_shared_memory - 更高级的共享内存
// ============================================================
// 老师傅说：
// 上面的 shared_memory_object 太原始了，
// 就像给你一块空地，你自己在上面盖房子。
//
// Boost.Interprocess 还提供了 managed_shared_memory，
// 相当于"带物业管理的共享内存"：
//   - 自带内存分配器（在共享内存里 new/delete）
//   - 可以构造对象、查找对象、销毁对象
//   - 支持在共享内存里用 STL 容器（vector, string, map...）
//
// 用起来方便多了，实际项目中推荐用这个。
// ============================================================

void demo_managed_shm() {
    cout << "\n=== 演示3：managed_shared_memory（带管理的共享内存） ===" << endl;

    const char* shm_name = "managed_shm_demo";

    try {
        // 清理旧的
        bip::shared_memory_object::remove(shm_name);

        // ---- 创建 managed shared memory ----
        // 第一个参数：创建方式
        // 第二个参数：共享内存名字
        // 第三个参数：大小（要比实际需要大一些，因为管理信息也要占空间）
        bip::managed_shared_memory segment(
            bip::create_only,
            shm_name,
            65536  // 64KB
        );

        cout << "managed_shared_memory 创建成功" << endl;

        // ---- 在共享内存中构造对象 ----
        // 相当于：在共享内存里 new 一个对象
        // 第一个参数是"名字"，用来以后查找
        int* shared_int = segment.construct<int>("my_counter")(42);
        cout << "在共享内存中构造 int，初始值: " << *shared_int << endl;

        // 也可以构造字符串（注意：要用 Boost.Interprocess 提供的 string，
        // 因为 std::string 内部用的是进程内的分配器，不能跨进程用）
        // 这里先放个简单的例子

        // ---- 查找对象 ----
        // 另一个进程可以通过名字找到这个对象
        // find 返回一个 pair：指针 + 是否找到
        auto result = segment.find<int>("my_counter");
        if (result.first) {
            cout << "查找到 my_counter，值: " << *result.first << endl;

            // 修改它
            *result.first = 100;
            cout << "修改后的值: " << *shared_int << endl;
        } else {
            cout << "没找到 my_counter" << endl;
        }

        // ---- 销毁对象 ----
        segment.destroy<int>("my_counter");
        cout << "对象已销毁" << endl;

        // 再次查找（应该找不到了）
        result = segment.find<int>("my_counter");
        cout << "销毁后查找: " << (result.first ? "还在" : "已删除") << endl;

        // 清理
        bip::shared_memory_object::remove(shm_name);
        cout << "共享内存已删除" << endl;

    } catch (const bip::interprocess_exception& e) {
        cerr << "出错: " << e.what() << endl;
        bip::shared_memory_object::remove(shm_name);
    }
}

// ============================================================
// 七、共享内存中的 STL 容器
// ============================================================
// 老师傅讲关键点：
// 想在共享内存里用 vector、string 这些容器？
// 直接用 std::vector 可不行，因为它内部的内存分配是在本进程堆上的，
// 另一个进程看不到。
//
// 所以 Boost.Interprocess 提供了：
//   - 自己的分配器（allocator）：从共享内存里分配内存
//   - 自己的容器：vector, string, deque, list, map, set...
//     （接口和 std 的几乎一样，只是分配器不同）
//
// 这样你就能在共享内存里放各种数据结构了，很爽吧？
// ============================================================

// 定义共享内存分配器类型
// 分配器是模板，指定分配的类型
typedef bip::allocator<int, bip::managed_shared_memory::segment_manager>
    ShmIntAllocator;

// 定义共享内存中的 vector
typedef bip::vector<int, ShmIntAllocator>
    ShmIntVector;

void demo_shm_vector() {
    cout << "\n=== 演示4：共享内存中的 vector 容器 ===" << endl;

    const char* shm_name = "shm_vector_demo";

    try {
        bip::shared_memory_object::remove(shm_name);

        bip::managed_shared_memory segment(
            bip::create_only,
            shm_name,
            65536  // 64KB
        );

        // 获取分配器实例
        ShmIntAllocator alloc_inst(segment.get_segment_manager());

        // 在共享内存中构造一个 vector
        ShmIntVector* vec = segment.construct<ShmIntVector>("my_vector")(alloc_inst);

        // 往里面塞数据（用法和 std::vector 一模一样！）
        for (int i = 0; i < 10; ++i) {
            vec->push_back(i * 10);
        }
        cout << "在共享内存 vector 中放入了 " << vec->size() << " 个元素" << endl;

        // 另一个进程中查找并读取
        auto result = segment.find<ShmIntVector>("my_vector");
        if (result.first) {
            cout << "查找到 vector，内容：";
            for (int val : *result.first) {
                cout << val << " ";
            }
            cout << endl;
        }

        // 清理
        segment.destroy<ShmIntVector>("my_vector");
        bip::shared_memory_object::remove(shm_name);

        cout << "共享内存 vector 演示完成。" << endl;
        cout << "看到没？用法和 std::vector 几乎一样，" << endl;
        cout << "只是分配器是从共享内存里分配的。" << endl;

    } catch (const bip::interprocess_exception& e) {
        cerr << "出错: " << e.what() << endl;
        bip::shared_memory_object::remove(shm_name);
    }
}

// ============================================================
// 八、消息队列（Message Queue）
// ============================================================
// 老师傅讲场景：
// 共享内存 + 同步，虽然快，但用起来麻烦。
// 如果你只是想发消息、收消息，不需要共享大量数据，
// 消息队列可能更合适。
//
// 消息队列就像一个"信箱"：
//   - 发送方把消息扔进队列
//   - 接收方从队列里取消息
//   - 队列有缓冲区，发的快收的慢也不怕（不超过容量就行）
//   - 消息自带优先级（可选）
//
// 优点：
//   - 收发解耦（发送方不用管谁收，接收方不用管谁发）
//   - 自带同步（不用自己写mutex）
//   - 支持优先级
// 缺点：
//   - 数据要拷贝（不如共享内存快）
//   - 消息大小有限制
// ============================================================

void demo_message_queue() {
    cout << "\n=== 演示5：消息队列 ===" << endl;

    const char* mq_name = "demo_mq";

    try {
        // 先删除旧的
        bip::message_queue::remove(mq_name);

        // ---- 创建消息队列 ----
        // 参数：创建方式、名字、最大消息数、单条消息最大大小
        bip::message_queue mq(
            bip::create_only,
            mq_name,
            10,          // 最多存10条消息
            256          // 每条消息最大256字节
        );

        cout << "消息队列创建成功" << endl;

        // ---- 发送消息 ----
        const char* msg1 = "第一条消息";
        const char* msg2 = "第二条消息";
        const char* msg3 = "高优先级消息";

        // send 参数：消息地址、消息大小、优先级
        mq.send(msg1, strlen(msg1) + 1, 0);   // 优先级 0（默认）
        cout << "发送: " << msg1 << " (优先级0)" << endl;

        mq.send(msg2, strlen(msg2) + 1, 0);
        cout << "发送: " << msg2 << " (优先级0)" << endl;

        mq.send(msg3, strlen(msg3) + 1, 10);  // 高优先级
        cout << "发送: " << msg3 << " (优先级10)" << endl;

        // ---- 接收消息 ----
        // 注意：高优先级的会先出来！
        char buffer[256];
        size_t recv_size;
        unsigned int priority;

        cout << "\n接收消息（高优先级先出）：" << endl;
        for (int i = 0; i < 3; ++i) {
            mq.receive(buffer, sizeof(buffer), recv_size, priority);
            cout << "  收到: " << buffer
                 << "（优先级: " << priority
                 << "，大小: " << recv_size << "）" << endl;
        }

        // ---- 非阻塞接收 ----
        // try_receive 如果没消息立即返回 false，不会阻塞
        bool got = mq.try_receive(buffer, sizeof(buffer), recv_size, priority);
        cout << "\n非阻塞接收: " << (got ? "有消息" : "队列空了") << endl;

        // 清理
        bip::message_queue::remove(mq_name);
        cout << "消息队列已删除" << endl;

    } catch (const bip::interprocess_exception& e) {
        cerr << "出错: " << e.what() << endl;
        bip::message_queue::remove(mq_name);
    }
}

// ============================================================
// 九、命名管道（Named Pipe）
// ============================================================
// 老师傅说：
// 命名管道（Windows 上叫 Named Pipe，Linux 上叫 FIFO）
// 也是一种IPC方式，特点是：
//   - 像文件一样有名字
//   - 字节流方式（和 socket 很像）
//   - 单向的（一个写一个读）
//   - 只能在同一台机器上用
//
// Boost.Interprocess 的 named_pipe 封装了跨平台的命名管道。
//
// 实际项目中命名管用得不是特别多，因为：
//   - 和 Unix domain socket 比，功能差不多
//   - 和消息队列比，没有结构化、没有优先级
//   - 和共享内存比，速度慢（数据要拷贝）
//
// 了解一下就行，真遇到了知道是啥。
// ============================================================

void demo_named_pipe() {
    cout << "\n=== 演示6：命名管道（Named Pipe） ===" << endl;

    // 注意：命名管道的完整演示需要两个进程
    // 这里只展示基本概念和API，实际使用需要两个程序

    cout << "命名管道（Named Pipe）是一种字节流式的IPC方式。" << endl;
    cout << endl;
    cout << "特点：" << endl;
    cout << "  - 像文件一样有名字，通过名字打开" << endl;
    cout << "  - 字节流传输（和 TCP socket 类似）" << endl;
    cout << "  - 单向：一端写，一端读" << endl;
    cout << "  - 只能本机使用，不能跨网络" << endl;
    cout << endl;
    cout << "Boost.Interprocess 中的相关类：" << endl;
    cout << "  - bip::named_pipe  - 命名管道" << endl;
    cout << "  - 支持 read/write 操作" << endl;
    cout << "  - 支持阻塞和非阻塞模式" << endl;
    cout << endl;
    cout << "适用场景：" << endl;
    cout << "  - 简单的命令式通信（发指令、收结果）" << endl;
    cout << "  - 流式数据传输（比如传日志）" << endl;
    cout << endl;
    cout << "（命名管道需要两个独立进程才能完整演示，这里就不跑了）" << endl;
}

// ============================================================
// 十、生产者-消费者模型（共享内存版）
// ============================================================
// 老师傅说：
// 学了这么多，咱们来个综合的：用共享内存 + 互斥锁 + 条件变量
// 实现一个进程间的生产者-消费者模型。
//
// 这个例子展示了实际项目中共享内存的典型用法：
//   - 一个共享缓冲区
//   - 生产者往里放数据
//   - 消费者从里取数据
//   - 用 mutex 保护缓冲区
//   - 用 condition_variable 做"满了等""空了等"
// ============================================================

// 共享内存中的环形缓冲区
const int BUFFER_SIZE = 5;

struct RingBuffer {
    bip::interprocess_mutex mutex;
    bip::interprocess_condition not_full;   // 缓冲区不满，可以写
    bip::interprocess_condition not_empty;  // 缓冲区不空，可以读

    int buffer[BUFFER_SIZE];
    int head;   // 写位置
    int tail;   // 读位置
    int count;  // 当前元素个数

    RingBuffer() : head(0), tail(0), count(0) {}
};

void demo_producer_consumer_shm() {
    cout << "\n=== 演示7：综合实战 - 共享内存生产者-消费者 ===" << endl;

    const char* shm_name = "prod_cons_shm";

    try {
        bip::shared_memory_object::remove(shm_name);

        bip::shared_memory_object shm(
            bip::open_or_create,
            shm_name,
            bip::read_write
        );
        shm.truncate(sizeof(RingBuffer));

        bip::mapped_region region(shm, bip::read_write);
        RingBuffer* rb = new (region.get_address()) RingBuffer();

        cout << "环形缓冲区已创建（容量: " << BUFFER_SIZE << "）" << endl;

        // 生产者（用线程模拟进程A）
        auto producer = [rb]() {
            for (int i = 1; i <= 10; ++i) {
                bip::scoped_lock<bip::interprocess_mutex> lock(rb->mutex);

                // 缓冲区满了，等消费者取走
                while (rb->count >= BUFFER_SIZE) {
                    cout << "  生产者：缓冲区满了，等待..." << endl;
                    rb->not_full.wait(lock);
                }

                // 放入数据
                rb->buffer[rb->head] = i;
                rb->head = (rb->head + 1) % BUFFER_SIZE;
                rb->count++;
                cout << "  生产者：放入 " << i << "，当前数量: " << rb->count << endl;

                // 通知消费者：有数据了
                rb->not_empty.notify_one();
            }
            cout << "  生产者：生产完毕" << endl;
        };

        // 消费者（用线程模拟进程B）
        auto consumer = [rb]() {
            int received = 0;
            while (received < 10) {
                bip::scoped_lock<bip::interprocess_mutex> lock(rb->mutex);

                // 缓冲区空了，等生产者放
                while (rb->count <= 0) {
                    cout << "    消费者：缓冲区空了，等待..." << endl;
                    rb->not_empty.wait(lock);
                }

                // 取出数据
                int val = rb->buffer[rb->tail];
                rb->tail = (rb->tail + 1) % BUFFER_SIZE;
                rb->count--;
                received++;
                cout << "    消费者：取出 " << val << "，当前数量: " << rb->count << endl;

                // 通知生产者：有位置了
                rb->not_full.notify_one();
            }
            cout << "    消费者：消费完毕" << endl;
        };

        thread prod(producer);
        this_thread::sleep_for(chrono::milliseconds(50));  // 让生产者先跑一点
        thread cons(consumer);

        prod.join();
        cons.join();

        cout << "生产者-消费者模型运行成功！" << endl;
        cout << "这就是共享内存 + 同步的典型用法。" << endl;

        // 清理
        rb->~RingBuffer();
        bip::shared_memory_object::remove(shm_name);

    } catch (const bip::interprocess_exception& e) {
        cerr << "出错: " << e.what() << endl;
        bip::shared_memory_object::remove(shm_name);
    }
}

// ============================================================
// 主函数
// ============================================================
int main() {
    cout << "============================================================" << endl;
    cout << "  并发编程进阶07：进程间通信（Boost.Interprocess）" << endl;
    cout << "============================================================" << endl;

    try {
        demo_basic_shared_memory();       // 基础共享内存
        demo_synchronized_shm();          // 带同步的共享内存
        demo_managed_shm();               // managed_shared_memory
        demo_shm_vector();                // 共享内存中的vector
        demo_message_queue();             // 消息队列
        demo_named_pipe();                // 命名管道（概念）
        demo_producer_consumer_shm();     // 生产者-消费者综合实战
    } catch (const exception& e) {
        cerr << "\n程序异常: " << e.what() << endl;
        cerr << "请确保已安装 Boost 库并正确链接。" << endl;
        return 1;
    }

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "徒弟啊，IPC 这东西，说难不难，说简单也不简单。" << endl;
    cout << "核心就一句话：不同的场景选不同的工具。" << endl;
    cout << "" << endl;
    cout << "1. 共享内存最快，但同步最麻烦。" << endl;
    cout << "   大量数据传输、对性能要求极高的场景用它。" << endl;
    cout << "   但互斥锁、条件变量、信号量这些同步机制得玩明白，" << endl;
    cout << "   不然死锁、竞态、数据撕裂，各种坑等着你。" << endl;
    cout << "" << endl;
    cout << "2. 消息队列最方便，自带同步、支持优先级。" << endl;
    cout << "   结构化消息、任务分发的场景很适合。" << endl;
    cout << "   缺点是数据要拷贝，大数据量下性能不如共享内存。" << endl;
    cout << "" << endl;
    cout << "3. Socket 最通用，跨机器、跨语言、跨平台。" << endl;
    cout << "   但也是最慢的，因为要走网络协议栈。" << endl;
    cout << "   分布式系统、微服务，基本都是 socket。" << endl;
    cout << "" << endl;
    cout << "4. Boost.Interprocess 是个好库，但也不是银弹。" << endl;
    cout << "   它封装得很好，但有些细节还是要注意：" << endl;
    cout << "   - 共享内存里不能放普通的 std 容器，要用 Boost 的版本" << endl;
    cout << "   - 共享内存在内核中，进程退出了不会自动删，记得手动清理" << endl;
    cout << "   - 指针不能直接放共享内存里（地址空间不一样），要用偏移量" << endl;
    cout << "" << endl;
    cout << "5. 实际项目中怎么选？我的经验：" << endl;
    cout << "   - 同一台机器 + 大数据量 + 性能要求高 → 共享内存" << endl;
    cout << "   - 同一台机器 + 小消息 + 结构化 → 消息队列" << endl;
    cout << "   - 跨机器 → socket（gRPC, ZeroMQ...）" << endl;
    cout << "   - 简单命令传递 → 管道" << endl;
    cout << "" << endl;
    cout << "记住：没有最好的IPC方式，只有最合适的。" << endl;
    cout << "根据场景选工具，别拿着锤子看什么都是钉子。" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习" << endl;
    cout << "============================================================" << endl;
    cout << "1. 写两个独立的程序（两个main函数）：" << endl;
    cout << "   - writer 程序：创建共享内存，写入一些数据" << endl;
    cout << "   - reader 程序：打开共享内存，读取并打印数据" << endl;
    cout << "   先运行 writer，再运行 reader，看看能不能读到数据。" << endl;
    cout << "" << endl;
    cout << "2. 在上面的基础上，加上 interprocess_mutex：" << endl;
    cout << "   writer 写的时候加锁，reader 读的时候也加锁，" << endl;
    cout << "   验证一下多进程下的互斥效果。" << endl;
    cout << "" << endl;
    cout << "3. 用消息队列实现一个简单的"任务分发系统"：" << endl;
    cout << "   - 一个发送者进程，发送带优先级的任务" << endl;
    cout << "   - 多个接收者进程，从队列里取任务执行" << endl;
    cout << "   观察高优先级任务是不是先被处理。" << endl;
    cout << "" << endl;
    cout << "4. 思考：为什么共享内存里不能直接存指针？" << endl;
    cout << "   如果我想在共享内存里实现一个链表，该怎么办？" << endl;
    cout << "   （提示：查一下 offset_ptr 和 相对地址）" << endl;
    cout << "" << endl;
    cout << "5. 查一下什么是"内存映射文件"（Memory-Mapped File）：" << endl;
    cout << "   它和共享内存有什么区别？" << endl;
    cout << "   什么情况下你会选择用内存映射文件而不是共享内存？" << endl;
    cout << "" << endl;
    cout << "6. 了解一下零拷贝（Zero-Copy）技术：" << endl;
    cout << "   为什么共享内存快？快在哪里？" << endl;
    cout << "   还有哪些零拷贝技术？" << endl;

    return 0;
}
