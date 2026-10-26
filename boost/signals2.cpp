/**
 * Boost.Signals2 信号槽机制 Demo
 *
 * 知识点覆盖：
 *   1. 信号槽基础：signal 定义、slot 连接、信号触发
 *   2. 线程安全信号：多线程环境下的信号发射与连接管理
 *   3. 信号组合器（Combiner）：自定义多个 slot 返回值的合并方式
 *   4. 连接管理：connection / scoped_connection / 连接分组
 *   5. Slot 生命周期管理：track 追踪 shared_ptr 对象生命周期
 *   6. 观察者模式实现：事件驱动架构的典型应用
 *   7. 连接分组与调用顺序：组号、connect_position（不是“优先级”）
 *   8. 自动断开机制：track + 弱引用实现安全的对象生命周期管理
 *
 * Boost 优势：
 *   - 线程安全：signals2 是信号库的线程安全版本（signals v1 不是）
 *   - 自动连接管理：slot 对象析构时自动断开连接（通过 track）
 *   - 灵活的组合器：自定义多 slot 返回值的聚合方式
 *   - 连接分组：用组号和插入位置精细控制 slot 调用顺序
 *   - scoped_connection：RAII 风格的连接管理
 *   - C++ 标准库中无对应功能
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 signals2.cpp -o signals2 -pthread   （Signals2 是头文件实现）
 *   （Signals2 是纯头文件库，但线程部分可能需要 pthread）
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <numeric>
#include <algorithm>

// ========== Boost 头文件说明 ==========
// <boost/signals2.hpp>              - Signals2 总头文件
// <boost/signals2/signal.hpp>       - signal 类模板
// <boost/signals2/connection.hpp>   - connection 连接管理
// <boost/signals2/scoped_connection.hpp> - scoped_connection 作用域连接
// <boost/signals2/trackable.hpp>    - trackable 基类（自动断开）
// <boost/signals2/slot.hpp>         - slot 类
// <boost/signals2/combiner.hpp>     - 组合器相关
#include <boost/signals2.hpp>

namespace sig = boost::signals2;

// ============================================================
// 一、信号槽基础
// ============================================================
// 信号（signal）是事件的发出者，槽（slot）是事件的处理者。
// 一个信号可以连接多个槽，发射信号时所有连接的槽都会被调用。

void basic_signals_demo() {
    std::cout << "\n--- 1. 信号槽基础 ---" << std::endl;

    // 1.1 基本用法：void 返回值的信号
    std::cout << "1.1 基本信号槽:" << std::endl;
    sig::signal<void(const std::string&)> message_signal;

    // 连接槽（lambda 表达式）
    message_signal.connect([](const std::string& msg) {
        std::cout << "  [Slot 1] 收到消息: " << msg << std::endl;
    });

    message_signal.connect([](const std::string& msg) {
        std::cout << "  [Slot 2] 收到消息 (大写): ";
        for (char c : msg) std::cout << (char)toupper(c);
        std::cout << std::endl;
    });

    // 发射信号
    std::cout << "  发射信号: Hello, Signals2!" << std::endl;
    message_signal("Hello, Signals2!");

    // 1.2 有返回值的信号 + 默认组合器（返回最后一个 slot 的值）
    std::cout << "\n1.2 有返回值的信号（默认返回最后一个）:" << std::endl;
    sig::signal<int(int, int)> math_signal;

    math_signal.connect([](int a, int b) {
        int sum = a + b;
        std::cout << "  [加法] " << a << " + " << b << " = " << sum << std::endl;
        return sum;
    });

    math_signal.connect([](int a, int b) {
        int product = a * b;
        std::cout << "  [乘法] " << a << " * " << b << " = " << product << std::endl;
        return product;
    });

    // 默认 combiner 是 optional_last_value，返回 boost::optional<int>
    auto result = math_signal(3, 4);
    std::cout << "  信号返回值（最后一个 slot）: "
              << (result ? std::to_string(*result) : "无返回值") << std::endl;

    // 1.3 连接数量查询
    std::cout << "\n1.3 连接管理:" << std::endl;
    std::cout << "  math_signal 连接数: " << math_signal.num_slots() << std::endl;
    std::cout << "  math_signal 是否为空: " << std::boolalpha << math_signal.empty() << std::endl;

    // 清空所有连接
    math_signal.disconnect_all_slots();
    std::cout << "  清空后连接数: " << math_signal.num_slots() << std::endl;
}

// ============================================================
// 二、信号组合器（Combiner）
// ============================================================
// 组合器决定如何聚合多个 slot 的返回值。
// 默认组合器是 optional_last_value（返回最后一个非空值）。
// 可以自定义组合器实现最大值、求和、收集所有结果等行为。

// 2.1 自定义组合器：收集所有返回值到 vector
template <typename T>
struct collect_all {
    using result_type = std::vector<T>;

    template <typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const {
        result_type results;
        while (first != last) {
            results.push_back(*first);
            ++first;
        }
        return results;
    }
};

// 2.2 自定义组合器：求最大值
struct max_value {
    using result_type = double;

    template <typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const {
        if (first == last) return 0.0;
        double max_val = *first;
        ++first;
        while (first != last) {
            if (*first > max_val) max_val = *first;
            ++first;
        }
        return max_val;
    }
};

// 2.3 自定义组合器：短路逻辑与（遇到 false 立即停止调用后续 slot）
struct logical_and {
    using result_type = bool;

    template <typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const {
        while (first != last) {
            if (!*first) return false;  // 短路：遇到 false 立即返回
            ++first;
        }
        return true;
    }
};

void combiner_demo() {
    std::cout << "\n--- 2. 信号组合器 ---" << std::endl;

    // 2.1 收集所有返回值
    std::cout << "2.1 collect_all 组合器（收集所有结果）:" << std::endl;
    sig::signal<double(double), collect_all<double>> sqrt_signal;

    sqrt_signal.connect([](double x) {
        double r = std::sqrt(x);
        std::cout << "  [sqrt] sqrt(" << x << ") = " << r << std::endl;
        return r;
    });

    sqrt_signal.connect([](double x) {
        double r = std::cbrt(x);
        std::cout << "  [cbrt] cbrt(" << x << ") = " << r << std::endl;
        return r;
    });

    sqrt_signal.connect([](double x) {
        double r = std::log(x);
        std::cout << "  [log] log(" << x << ") = " << r << std::endl;
        return r;
    });

    auto results = sqrt_signal(64.0);
    std::cout << "  所有结果: [";
    for (size_t i = 0; i < results.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << results[i];
    }
    std::cout << "]" << std::endl;

    // 2.2 最大值组合器
    std::cout << "\n2.2 max_value 组合器:" << std::endl;
    sig::signal<double(double), max_value> max_signal;

    max_signal.connect([](double x) {
        double r = x * 2;
        std::cout << "  [*2] " << x << " * 2 = " << r << std::endl;
        return r;
    });

    max_signal.connect([](double x) {
        double r = x * x;
        std::cout << "  [^2] " << x << " ^ 2 = " << r << std::endl;
        return r;
    });

    max_signal.connect([](double x) {
        double r = x + 50;
        std::cout << "  [+50] " << x << " + 50 = " << r << std::endl;
        return r;
    });

    double max_result = max_signal(10.0);
    std::cout << "  最大值: " << max_result << std::endl;

    // 2.3 短路逻辑与组合器
    std::cout << "\n2.3 logical_and 短路组合器:" << std::endl;
    sig::signal<bool(int), logical_and> validate_signal;

    validate_signal.connect([](int x) {
        bool ok = x > 0;
        std::cout << "  [检查1] x > 0: " << std::boolalpha << ok << std::endl;
        return ok;
    });

    validate_signal.connect([](int x) {
        bool ok = x < 100;
        std::cout << "  [检查2] x < 100: " << std::boolalpha << ok << std::endl;
        return ok;
    });

    validate_signal.connect([](int x) {
        bool ok = x % 2 == 0;
        std::cout << "  [检查3] x 是偶数: " << std::boolalpha << ok << std::endl;
        return ok;
    });

    std::cout << "  验证 5（三个检查都会执行）:" << std::endl;
    bool r1 = validate_signal(5);
    std::cout << "  结果: " << std::boolalpha << r1
              << " (检查3失败：5 不是偶数)" << std::endl;

    std::cout << "  验证 150（检查2失败后短路，检查3不执行）:" << std::endl;
    bool r3 = validate_signal(150);
    std::cout << "  结果: " << std::boolalpha << r3
              << " (检查2失败，检查3未执行)" << std::endl;

    std::cout << "  验证 50:" << std::endl;
    bool r2 = validate_signal(50);
    std::cout << "  结果: " << std::boolalpha << r2 << std::endl;
}

// ============================================================
// 三、连接管理
// ============================================================
// connection 对象表示信号与槽的连接，可以手动断开。
// scoped_connection 是 RAII 风格的连接，离开作用域自动断开。
// 连接分组可以批量断开同一组的连接。

void connection_management_demo() {
    std::cout << "\n--- 3. 连接管理 ---" << std::endl;

    sig::signal<void()> sig;

    // 3.1 connection：手动管理连接
    std::cout << "3.1 connection 手动断开:" << std::endl;
    sig::connection conn1 = sig.connect([]() {
        std::cout << "  Slot 1 被调用" << std::endl;
    });
    sig::connection conn2 = sig.connect([]() {
        std::cout << "  Slot 2 被调用" << std::endl;
    });

    std::cout << "  第一次发射（2个slot）:" << std::endl;
    sig();

    conn1.disconnect();  // 手动断开 slot 1
    std::cout << "  断开 Slot 1 后发射:" << std::endl;
    sig();

    std::cout << "  conn1 是否连接: " << std::boolalpha << conn1.connected() << std::endl;
    std::cout << "  conn2 是否连接: " << conn2.connected() << std::endl;

    // 3.2 scoped_connection：作用域连接（离开作用域自动断开）
    std::cout << "\n3.2 scoped_connection 作用域连接:" << std::endl;
    {
        sig::scoped_connection scoped_conn = sig.connect([]() {
            std::cout << "  [Scoped Slot] 被调用" << std::endl;
        });
        std::cout << "  作用域内发射:" << std::endl;
        sig();
    }  // scoped_conn 析构，自动断开

    std::cout << "  离开作用域后发射（只有 Slot 2）:" << std::endl;
    sig();

    // 3.3 连接分组：按组管理连接
    std::cout << "\n3.3 连接分组:" << std::endl;
    sig::signal<void(int)> group_signal;

    // 使用 connect 时指定组号（int 类型）
    group_signal.connect(1, [](int x) {
        std::cout << "  [组1] 收到: " << x << std::endl;
    });
    group_signal.connect(1, [](int x) {
        std::cout << "  [组1-2] 收到: " << x << std::endl;
    });
    group_signal.connect(2, [](int x) {
        std::cout << "  [组2] 收到: " << x << std::endl;
    });

    std::cout << "  发射信号（所有组）:" << std::endl;
    group_signal(42);

    // 断开组 1 的所有连接
    group_signal.disconnect(1);
    std::cout << "  断开组 1 后发射:" << std::endl;
    group_signal(42);
}

// ============================================================
// 四、调用顺序（分组，不是“优先级”）
// ============================================================
// Signals2 没有 priority 参数，控制调用顺序靠“组号”和 connect_position：
//   - 组号小的组先调用；同一组内默认按 connect 的先后顺序调用。
//   - connect(group, slot, sig::at_front) 可以把 slot 插到该组最前面。
//   - connect(slot, sig::at_front) 插入“最前面的未分组区”，比所有组都早。
//   - connect(slot) 默认 at_back，插入“最后面的未分组区”，比所有组都晚。
//
// 注意 at_front / at_back 是 connect_position 枚举（at_back=0, at_front=1），
// 必须作为第二个/第三个参数传给 connect。若写成 connect(sig::at_front, slot)，
// 它会匹配到 connect(group, slot)，被当成“组号 1”，不是“放到最前”！

void priority_demo() {
    std::cout << "\n--- 4. 分组与调用顺序（Signals2 无优先级参数） ---" << std::endl;

    sig::signal<void()> ordered_signal;

    // 未分组：默认 at_back 在所有组之后；at_front 在所有组之前
    ordered_signal.connect([]() {
        std::cout << "  [未分组-默认] 所有组之后" << std::endl;
    });
    ordered_signal.connect([]() {
        std::cout << "  [未分组-at_front] 所有组之前" << std::endl;
    }, sig::at_front);

    // 分组：组号小的先调用
    ordered_signal.connect(5, []() {
        std::cout << "  [组5] 在组0之后" << std::endl;
    });
    ordered_signal.connect(0, []() {
        std::cout << "  [组0-A] 最先调用" << std::endl;
    });
    ordered_signal.connect(0, []() {
        std::cout << "  [组0-B] 同组按连接顺序" << std::endl;
    });
    ordered_signal.connect(0, []() {
        std::cout << "  [组0-C-at_front] 插到组0最前" << std::endl;
    }, sig::at_front);

    std::cout << "  调用顺序（未分组-at_front → 组0 → 组5 → 未分组-默认）:"
              << std::endl;
    ordered_signal();
}

// ============================================================
// 五、Slot 生命周期管理（track 机制）
// ============================================================
// track 是 Signals2 的一个强大功能：
//   - slot 可以追踪一个 shared_ptr 对象
//   - 当被追踪的对象失效时，连接自动断开
//   - 这样可以避免调用已经析构的对象的成员函数
//
// 这是观察者模式中安全管理对象生命周期的关键机制。

class DataProcessor : public std::enable_shared_from_this<DataProcessor> {
public:
    DataProcessor(const std::string& name) : name_(name) {
        std::cout << "  [DataProcessor] " << name_ << " 构造" << std::endl;
    }

    ~DataProcessor() {
        std::cout << "  [DataProcessor] " << name_ << " 析构" << std::endl;
    }

    void process(int data) {
        std::cout << "  [" << name_ << "] 处理数据: " << data << std::endl;
    }

private:
    std::string name_;
};

void track_lifetime_demo() {
    std::cout << "\n--- 5. Slot 生命周期管理（track 机制） ---" << std::endl;

    sig::signal<void(int)> data_signal;

    // 5.1 使用 track 追踪 shared_ptr
    std::cout << "5.1 track 追踪 shared_ptr:" << std::endl;

    auto processor1 = std::make_shared<DataProcessor>("处理器A");
    auto processor2 = std::make_shared<DataProcessor>("处理器B");

    // slot 追踪 processor1，processor1 失效时自动断开。
    // 注意 lambda 里必须捕获 weak_ptr，否则 shared_ptr 被 slot 持有，
    // 对象永远不会析构，track 也就永远不会生效。
    auto weak1 = std::weak_ptr<DataProcessor>(processor1);
    data_signal.connect(
        sig::slot<void(int)>([weak1](int data) {
            if (auto p = weak1.lock()) {
                p->process(data);
            }
        }).track_foreign(processor1)
    );

    // 另一种写法：track_foreign 直接追踪 weak_ptr
    auto weak2 = std::weak_ptr<DataProcessor>(processor2);
    data_signal.connect(
        sig::slot<void(int)>([weak2](int data) {
            if (auto p = weak2.lock()) {
                p->process(data);
            }
        }).track_foreign(weak2)
    );

    std::cout << "  两个处理器都存活时发射信号:" << std::endl;
    data_signal(100);

    // 销毁 processor1
    std::cout << "  销毁处理器A..." << std::endl;
    processor1.reset();

    std::cout << "  只有处理器B存活时发射信号:" << std::endl;
    data_signal(200);  // 处理器A的slot已自动断开，不会被调用

    // 5.2 scoped_connection 与对象生命周期
    std::cout << "\n5.2 scoped_connection 管理成员函数连接:" << std::endl;

    class Observer {
    public:
        Observer(const std::string& name, sig::signal<void()>& sig)
            : name_(name) {
            // scoped_connection 作为成员，对象析构时自动断开
            conn_ = sig.connect([this]() {
                std::cout << "  [" << name_ << "] 收到通知" << std::endl;
            });
            std::cout << "  [Observer] " << name_ << " 创建" << std::endl;
        }
        ~Observer() {
            std::cout << "  [Observer] " << name_ << " 销毁（自动断开连接）" << std::endl;
        }
    private:
        std::string name_;
        sig::scoped_connection conn_;
    };

    sig::signal<void()> event_signal;
    {
        Observer obs1("观察者1", event_signal);
        Observer obs2("观察者2", event_signal);

        std::cout << "  发射事件:" << std::endl;
        event_signal();
    }  // obs1, obs2 析构，连接自动断开

    std::cout << "  观察者销毁后发射信号（无响应）:" << std::endl;
    event_signal();
    std::cout << "  （无输出，因为所有连接已自动断开）" << std::endl;
}

// ============================================================
// 六、线程安全信号
// ============================================================
// Signals2 的线程安全保证：
//   - 多线程同时调用信号发射是安全的
//   - 多线程同时连接/断开连接是安全的
//   - 信号发射过程中连接/断开也是安全的
//
// 但注意：slot 函数本身的线程安全需要用户保证。

class ThreadSafeCounter {
public:
    ThreadSafeCounter() : count_(0) {}

    void increment(int delta) {
        std::lock_guard<std::mutex> lock(mtx_);
        count_ += delta;
    }

    int value() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return count_;
    }

private:
    mutable std::mutex mtx_;
    int count_;
};

void thread_safety_demo() {
    std::cout << "\n--- 6. 线程安全信号 ---" << std::endl;

    // signals2 默认就是线程安全的（使用 mutex 保护内部数据结构）
    sig::signal<void(int)> increment_signal;

    auto counter = std::make_shared<ThreadSafeCounter>();

    // 连接多个 slot
    for (int i = 0; i < 3; ++i) {
        increment_signal.connect(
            [counter, i](int delta) {
                counter->increment(delta);
            }
        );
    }

    // 多线程同时发射信号
    const int num_threads = 5;
    const int increments_per_thread = 1000;
    std::vector<std::thread> threads;

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&increment_signal, increments_per_thread, i]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                increment_signal(1);  // 多线程同时发射信号是安全的
            }
        });
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 预期：5 线程 * 1000 次 * 3 个 slot = 15000
    int expected = num_threads * increments_per_thread * 3;
    std::cout << "  线程数: " << num_threads << std::endl;
    std::cout << "  每线程发射次数: " << increments_per_thread << std::endl;
    std::cout << "  Slot 数: 3" << std::endl;
    std::cout << "  预期计数: " << expected << std::endl;
    std::cout << "  实际计数: " << counter->value() << std::endl;
    std::cout << "  耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "  线程安全: "
              << (counter->value() == expected ? "验证通过" : "验证失败")
              << std::endl;
}

// ============================================================
// 七、观察者模式实战：事件总线
// ============================================================
// 使用 signals2 实现一个简单的事件总线（Event Bus），
// 这是观察者模式在实际项目中的典型应用。

enum class EventType {
    UserLoggedIn,
    UserLoggedOut,
    DataUpdated,
    ErrorOccurred
};

struct Event {
    EventType type;
    std::string message;
    std::string source;
};

class EventBus {
public:
    using EventHandler = std::function<void(const Event&)>;
    using Connection = sig::connection;

    // 订阅指定类型的事件
    Connection subscribe(EventType type, EventHandler handler) {
        return signals_[static_cast<int>(type)].connect(std::move(handler));
    }

    // 发布事件
    void publish(const Event& event) {
        // 先通知订阅了该类型的处理器，再通知全局监听器
        signals_[static_cast<int>(event.type)](event);
        all_events_signal_(event);
    }

    // 订阅所有事件
    Connection subscribe_all(EventHandler handler) {
        return all_events_signal_.connect(std::move(handler));
    }

private:
    // 每种事件类型一个 signal
    sig::signal<void(const Event&)> signals_[4];
    sig::signal<void(const Event&)> all_events_signal_;
};

void event_bus_demo() {
    std::cout << "\n--- 7. 观察者模式实战：事件总线 ---" << std::endl;

    EventBus bus;

    // 订阅用户登录事件
    auto login_conn = bus.subscribe(EventType::UserLoggedIn, [](const Event& e) {
        std::cout << "  [登录模块] 用户登录: " << e.message
                  << " (来自 " << e.source << ")" << std::endl;
    });

    // 订阅错误事件
    auto error_conn = bus.subscribe(EventType::ErrorOccurred, [](const Event& e) {
        std::cout << "  [错误处理] 发生错误: " << e.message << std::endl;
    });

    // 订阅数据更新事件
    auto data_conn = bus.subscribe(EventType::DataUpdated, [](const Event& e) {
        std::cout << "  [数据模块] 数据已更新: " << e.message << std::endl;
    });

    // 订阅所有事件（全局监听）
    auto all_conn = bus.subscribe_all([](const Event& e) {
        std::cout << "  [全局监听] " << e.source << ": " << e.message << std::endl;
    });

    // 发布一些事件
    std::cout << "  发布事件流:" << std::endl;
    bus.publish({EventType::UserLoggedIn, "alice", "web_app"});
    bus.publish({EventType::DataUpdated, "用户数据已同步", "sync_service"});
    bus.publish({EventType::ErrorOccurred, "数据库连接超时", "db_manager"});
    bus.publish({EventType::UserLoggedOut, "alice", "web_app"});

    // 断开登录事件订阅
    login_conn.disconnect();
    std::cout << "\n  断开登录订阅后发布登录事件:" << std::endl;
    bus.publish({EventType::UserLoggedIn, "bob", "mobile_app"});
    std::cout << "  （登录模块无响应，因为已断开；全局监听仍然收到）" << std::endl;

    all_conn.disconnect();
    std::cout << "\n  断开全局监听后再发布事件:" << std::endl;
    bus.publish({EventType::DataUpdated, "静默更新", "background"});
    std::cout << "  （全局监听无响应）" << std::endl;
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.Signals2 信号槽机制 Demo ==========" << std::endl;

    // 1. 基础用法
    basic_signals_demo();

    // 2. 组合器
    combiner_demo();

    // 3. 连接管理
    connection_management_demo();

    // 4. 分组与调用顺序
    priority_demo();

    // 5. 生命周期管理
    track_lifetime_demo();

    // 6. 线程安全
    thread_safety_demo();

    // 7. 观察者模式实战
    event_bus_demo();

    // --------------------------------------------------------
    // 总结
    // --------------------------------------------------------
    std::cout << "\n--- Boost.Signals2 核心要点总结 ---" << std::endl;
    std::cout << "  1. 信号槽机制：一个信号可连接多个槽，一对多通信" << std::endl;
    std::cout << "  2. 线程安全：多线程并发发射/连接都是安全的" << std::endl;
    std::cout << "  3. 组合器：自定义多 slot 返回值的聚合方式" << std::endl;
    std::cout << "  4. 连接管理：connection / scoped_connection / 分组" << std::endl;
    std::cout << "  5. track 机制：自动追踪对象生命周期，安全断开" << std::endl;
    std::cout << "  6. 调用顺序：用连接分组和 connect_position 控制（signals2 没有 priority 参数）" << std::endl;
    std::cout << "  7. 典型应用：事件总线、观察者模式、插件架构" << std::endl;
    std::cout << "  8. C++ 标准库无对应功能，Boost 是首选方案" << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
