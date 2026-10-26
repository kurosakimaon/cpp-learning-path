/**
 * 观察者模式 (Observer Pattern)
 *
 * 【模式意图】
 * 定义对象间的一种一对多的依赖关系，当一个对象的状态发生改变时，所有依赖于它的对象
 * 都得到通知并被自动更新。
 *
 * 【模式结构】
 * - Subject（主题/被观察者）：
 *   - 知道它的观察者
 *   - 提供注册和删除观察者的接口
 * - Observer（观察者）：
 *   - 为那些在目标发生改变时需获得通知的对象定义一个更新接口
 * - ConcreteSubject（具体主题）：
 *   - 将有关状态存入各ConcreteObserver对象
 *   - 当它的状态发生改变时，向它的各个观察者发出通知
 * - ConcreteObserver（具体观察者）：
 *   - 维护一个指向ConcreteSubject对象的引用
 *   - 存储有关状态，这些状态应与目标的状态保持一致
 *   - 实现Observer的更新接口以使自身状态与主题的状态保持一致
 *
 * 两种模型：
 * - 推模型(Push)：主题向观察者推送详细的改变信息（不管观察者是否需要）
 * - 拉模型(Pull)：主题只通知"我变了"，观察者自己决定拉取什么信息
 *
 * 【适用场景】
 * 1. 一个抽象模型有两个方面，其中一个方面依赖于另一个方面
 * 2. 一个对象的改变需要其他对象同时改变，而它不知道具体有多少对象有待改变
 * 3. 一个对象必须通知其他对象，而它又不能假定其他对象是谁
 *
 * 【优点】
 * 1. 主题和观察者之间是抽象耦合，符合开闭原则
 * 2. 支持广播通信，主题会向所有已注册的观察者发送通知
 * 3. 可以在运行时动态添加/删除观察者
 *
 * 【缺点】
 * 1. 如果一个主题有很多直接和间接的观察者，通知会花费很多时间
 * 2. 如果观察者和主题之间有循环依赖，可能导致系统崩溃
 * 3. 观察者只知道主题变了，但不知道是怎么变的（推模型可以解决）
 *
 * 【现代C++实现方式】
 * 1. 经典观察者：抽象基类+虚函数
 * 2. 信号槽机制：类似Qt的signal/slot
 * 3. weak_ptr生命周期管理：解决悬空指针问题
 * 4. 线程安全观察者：多线程环境下的安全实现
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <cassert>
#include <sstream>

// ============================================================
// 推模型 vs 拉模型
// ============================================================

// --- 推模型 ---
// 观察者接口：更新时接收具体数据
class IPushObserver {
public:
    virtual ~IPushObserver() = default;
    // 推模型：把更新的数据直接推送给观察者
    virtual void onUpdate(const std::string& data) = 0;
};

// 主题接口（推模型）
class IPushSubject {
public:
    virtual ~IPushSubject() = default;
    virtual void attach(std::shared_ptr<IPushObserver> observer) = 0;
    virtual void detach(std::shared_ptr<IPushObserver> observer) = 0;
    virtual void notify() = 0;
};

// 具体观察者：显示设备
class PushDisplay : public IPushObserver {
public:
    explicit PushDisplay(std::string name) : name_(std::move(name)) {}

    void onUpdate(const std::string& data) override {
        std::cout << "  [PushDisplay " << name_ << "] 收到推送数据: " << data << std::endl;
        lastData_ = data;
    }

private:
    std::string name_;
    std::string lastData_;
};

// 具体主题：气象站
class WeatherStation_Push : public IPushSubject {
public:
    void attach(std::shared_ptr<IPushObserver> observer) override {
        observers_.push_back(observer);
    }

    void detach(std::shared_ptr<IPushObserver> observer) override {
        observers_.remove(observer);
    }

    void notify() override {
        std::string data = "温度: " + std::to_string(temperature_) + "°C, "
                          + "湿度: " + std::to_string(humidity_) + "%";
        // 先拷贝一份观察者列表：如果某个观察者在 onUpdate 里
        // 调用 detach，直接遍历原 list 会使迭代器失效。
        auto snapshot = observers_;
        for (auto& observer : snapshot) {
            observer->onUpdate(data);
        }
    }

    void setMeasurements(float temp, float humidity) {
        temperature_ = temp;
        humidity_ = humidity;
        std::cout << "\n[WeatherStation] 数据更新: 温度=" << temp
                  << "°C, 湿度=" << humidity << "%" << std::endl;
        notify();
    }

private:
    std::list<std::shared_ptr<IPushObserver>> observers_;
    float temperature_ = 0.0f;
    float humidity_ = 0.0f;
};

// --- 拉模型 ---
// 拉模型：主题只通知"我变了"，观察者自己决定拉取什么
class IPullObserver {
public:
    virtual ~IPullObserver() = default;
    // 拉模型：只通知主题指针，观察者自己拉取需要的数据
    virtual void onUpdate(class IPullSubject* subject) = 0;
};

class IPullSubject {
public:
    virtual ~IPullSubject() = default;
    virtual void attach(std::shared_ptr<IPullObserver> observer) = 0;
    virtual void detach(std::shared_ptr<IPullObserver> observer) = 0;
    virtual void notify() = 0;
    virtual float getTemperature() const = 0;
    virtual float getHumidity() const = 0;
    virtual float getPressure() const = 0;
};

// 具体观察者：只关心温度的设备
class TemperatureDisplay : public IPullObserver {
public:
    explicit TemperatureDisplay(std::string name) : name_(std::move(name)) {}

    void onUpdate(IPullSubject* subject) override {
        // 观察者主动拉取自己关心的数据
        float temp = subject->getTemperature();
        std::cout << "  [TemperatureDisplay " << name_
                  << "] 拉取温度: " << temp << "°C" << std::endl;
    }

private:
    std::string name_;
};

// 具体观察者：关心温度和湿度的设备
class WeatherDisplay : public IPullObserver {
public:
    explicit WeatherDisplay(std::string name) : name_(std::move(name)) {}

    void onUpdate(IPullSubject* subject) override {
        float temp = subject->getTemperature();
        float humidity = subject->getHumidity();
        std::cout << "  [WeatherDisplay " << name_
                  << "] 拉取温度: " << temp << "°C, 湿度: " << humidity << "%" << std::endl;
    }

private:
    std::string name_;
};

// 具体主题：气象站（拉模型）
class WeatherStation_Pull : public IPullSubject {
public:
    void attach(std::shared_ptr<IPullObserver> observer) override {
        observers_.push_back(observer);
    }

    void detach(std::shared_ptr<IPullObserver> observer) override {
        observers_.remove(observer);
    }

    void notify() override {
        // 只通知，不推送数据；同样先拷贝列表，避免回调中 detach 导致迭代器失效
        auto snapshot = observers_;
        for (auto& observer : snapshot) {
            observer->onUpdate(this);
        }
    }

    void setMeasurements(float temp, float humidity, float pressure) {
        temperature_ = temp;
        humidity_ = humidity;
        pressure_ = pressure;
        std::cout << "\n[WeatherStation] 数据更新" << std::endl;
        notify();
    }

    float getTemperature() const override { return temperature_; }
    float getHumidity() const override { return humidity_; }
    float getPressure() const override { return pressure_; }

private:
    std::list<std::shared_ptr<IPullObserver>> observers_;
    float temperature_ = 0.0f;
    float humidity_ = 0.0f;
    float pressure_ = 0.0f;
};

// ============================================================
// 2. 信号槽实现（现代C++风格）
// ============================================================

/**
 * 信号槽机制：一种更灵活的观察者模式实现。
 * 信号(Signal)相当于主题，槽(Slot)相当于观察者的回调函数。
 *
 * 优点：
 * - 不需要观察者继承特定的接口
 * - 可以连接lambda、函数对象、成员函数等任意可调用对象
 * - 更符合现代C++的函数式编程风格
 *
 * 这是Qt信号槽机制的简化实现。
 */

// 信号连接标识
class Connection {
public:
    Connection() = default;
    explicit Connection(int id) : id_(id) {}
    int id() const { return id_; }
    bool isValid() const { return id_ >= 0; }
private:
    int id_ = -1;
};

// 信号类（主题）
template <typename... Args>
class Signal {
public:
    using Slot = std::function<void(Args...)>;

    // 连接一个槽（注册观察者）
    Connection connect(Slot slot) {
        std::lock_guard<std::mutex> lock(mutex_);
        int id = nextId_++;
        slots_.emplace_back(id, std::move(slot));
        return Connection(id);
    }

    // 断开连接
    void disconnect(Connection conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_.remove_if([id = conn.id()](const auto& pair) {
            return pair.first == id;
        });
    }

    // 发射信号（通知所有观察者）
    void emit(Args... args) {
        // 先拷贝一份，防止回调中修改slots_导致迭代器失效
        std::list<std::pair<int, Slot>> slotsCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            slotsCopy = slots_;
        }

        for (auto& pair : slotsCopy) {
            pair.second(args...);
        }
    }

    // 获取连接数
    size_t getSlotCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return slots_.size();
    }

private:
    mutable std::mutex mutex_;
    std::list<std::pair<int, Slot>> slots_;
    int nextId_ = 0;
};

// 示例：按钮类，使用信号槽
class Button {
public:
    Signal<const std::string&> onClicked;  // 点击信号

    void click() {
        std::cout << "\n[Button] 按钮被点击" << std::endl;
        onClicked.emit("确定按钮");
    }
};

// ============================================================
// 3. weak_ptr生命周期管理
// ============================================================

/**
 * 观察者模式的一个常见问题：观察者对象被销毁后，主题仍然持有其引用，
 * 导致悬空指针或内存泄漏。
 *
 * 使用weak_ptr可以优雅地解决这个问题：
 * - 主题持有观察者的weak_ptr（不增加引用计数）
 * - 通知前先检查weak_ptr是否还有效
 * - 观察者销毁后自动从主题中"消失"
 */

class IWeakObserver {
public:
    virtual ~IWeakObserver() = default;
    virtual void onEvent(int eventId, const std::string& data) = 0;
};

class WeakSubject {
public:
    void addObserver(std::weak_ptr<IWeakObserver> observer) {
        // 清理已过期的观察者
        cleanupExpired();
        observers_.push_back(std::move(observer));
    }

    void notify(int eventId, const std::string& data) {
        cleanupExpired();
        std::cout << "  [WeakSubject] 通知 " << observers_.size()
                  << " 个观察者 (事件ID: " << eventId << ")" << std::endl;

        for (auto& weakObs : observers_) {
            if (auto observer = weakObs.lock()) {
                observer->onEvent(eventId, data);
            }
        }
    }

    size_t getObserverCount() {
        cleanupExpired();
        return observers_.size();
    }

private:
    void cleanupExpired() {
        observers_.remove_if([](const std::weak_ptr<IWeakObserver>& weak) {
            return weak.expired();
        });
    }

    std::list<std::weak_ptr<IWeakObserver>> observers_;
};

// 具体观察者
class LogObserver : public IWeakObserver {
public:
    explicit LogObserver(std::string name) : name_(std::move(name)) {}

    void onEvent(int eventId, const std::string& data) override {
        std::cout << "    [LogObserver " << name_
                  << "] 收到事件 " << eventId << ": " << data << std::endl;
    }

    ~LogObserver() {
        std::cout << "  [LogObserver " << name_ << "] 被销毁" << std::endl;
    }

private:
    std::string name_;
};

// ============================================================
// 4. 线程安全的观察者
// ============================================================

/**
 * 多线程环境下的观察者模式需要考虑：
 * 1. 注册/注销/通知的线程安全（互斥锁保护）
 * 2. 通知时不能持有锁（避免死锁）
 * 3. 观察者可能在通知过程中被销毁（weak_ptr解决）
 * 4. 防止在回调中修改观察者列表导致迭代器失效
 */

template <typename EventData>
class ThreadSafeSubject {
public:
    using ObserverFunc = std::function<void(const EventData&)>;

    Connection subscribe(ObserverFunc observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        int id = nextId_++;
        observers_.emplace_back(id, std::move(observer));
        return Connection(id);
    }

    void unsubscribe(Connection conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.remove_if([id = conn.id()](const auto& pair) {
            return pair.first == id;
        });
    }

    void publish(const EventData& data) {
        // 先拷贝一份观察者列表，然后释放锁
        // 这样可以避免：
        // 1. 回调中调用subscribe/unsubscribe导致死锁
        // 2. 迭代器失效
        std::vector<ObserverFunc> observersCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observersCopy.reserve(observers_.size());
            for (auto& pair : observers_) {
                observersCopy.push_back(pair.second);
            }
        }

        // 在锁外执行回调
        for (auto& observer : observersCopy) {
            observer(data);
        }
    }

    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }

private:
    mutable std::mutex mutex_;
    std::list<std::pair<int, ObserverFunc>> observers_;
    int nextId_ = 0;
};

// 事件数据
struct NewsEvent {
    std::string category;
    std::string headline;
    std::string content;
};

// ============================================================
// 测试代码
// ============================================================

void testPushVsPull() {
    std::cout << "\n========== 推模型 vs 拉模型 ==========" << std::endl;

    // 推模型
    std::cout << "\n--- 推模型（Push） ---" << std::endl;
    WeatherStation_Push pushStation;
    auto display1 = std::make_shared<PushDisplay>("显示屏A");
    auto display2 = std::make_shared<PushDisplay>("显示屏B");
    pushStation.attach(display1);
    pushStation.attach(display2);
    pushStation.setMeasurements(25.5f, 60.0f);

    // 拉模型
    std::cout << "\n--- 拉模型（Pull） ---" << std::endl;
    WeatherStation_Pull pullStation;
    auto tempDisplay = std::make_shared<TemperatureDisplay>("温度表");
    auto weatherDisplay = std::make_shared<WeatherDisplay>("天气预报屏");
    pullStation.attach(tempDisplay);
    pullStation.attach(weatherDisplay);
    pullStation.setMeasurements(28.0f, 70.0f, 1013.0f);

    std::cout << "\n  推模型：主题推送所有数据，观察者被动接收" << std::endl;
    std::cout << "  拉模型：主题只通知，观察者按需拉取（更灵活）" << std::endl;
}

void testSignalSlot() {
    std::cout << "\n========== 信号槽机制测试 ==========" << std::endl;

    Button button;

    // 连接lambda
    auto conn1 = button.onClicked.connect([](const std::string& name) {
        std::cout << "  [Lambda槽] 按钮 " << name << " 被点击了" << std::endl;
    });

    // 连接函数对象
    struct Logger {
        void operator()(const std::string& name) {
            std::cout << "  [Logger槽] 记录日志: " << name << " clicked" << std::endl;
        }
    };
    [[maybe_unused]] auto conn2 = button.onClicked.connect(Logger{});

    // 点击按钮
    button.click();

    // 断开一个连接
    std::cout << "\n--- 断开第一个连接后 ---" << std::endl;
    button.onClicked.disconnect(conn1);
    button.click();

    std::cout << "\n  信号槽的优势：不需要继承特定接口，灵活且低耦合" << std::endl;
}

void testWeakPtrLifecycle() {
    std::cout << "\n========== weak_ptr生命周期管理测试 ==========" << std::endl;

    WeakSubject subject;

    {
        // 在作用域内创建观察者
        auto obs1 = std::make_shared<LogObserver>("观察者1");
        auto obs2 = std::make_shared<LogObserver>("观察者2");

        subject.addObserver(obs1);
        subject.addObserver(obs2);

        std::cout << "\n  观察者数量: " << subject.getObserverCount() << std::endl;
        subject.notify(1, "第一条消息");

        // obs3在作用域结束时会被销毁
        auto obs3 = std::make_shared<LogObserver>("观察者3");
        subject.addObserver(obs3);

        std::cout << "\n  观察者数量: " << subject.getObserverCount() << std::endl;
        subject.notify(2, "第二条消息");

        // obs3离开作用域后被销毁
        std::cout << "\n  (obs3离开作用域...)" << std::endl;
    }

    // 此时所有观察者都已被销毁
    std::cout << "\n  观察者数量（清理后）: " << subject.getObserverCount() << std::endl;
    subject.notify(3, "第三条消息");
    std::cout << "  (没有观察者收到通知)" << std::endl;
}

void testThreadSafeObserver() {
    std::cout << "\n========== 线程安全观察者测试 ==========" << std::endl;

    ThreadSafeSubject<NewsEvent> newsService;

    // 订阅者1
    auto sub1 = newsService.subscribe([](const NewsEvent& news) {
        std::cout << "  [订阅者1] 收到 " << news.category << " 新闻: "
                  << news.headline << std::endl;
    });

    // 订阅者2
    // Connection 只是订阅标识；销毁这个句柄不会自动退订，
    // 需要显式调用 unsubscribe(sub2)
    [[maybe_unused]] auto sub2 = newsService.subscribe([](const NewsEvent& news) {
        std::cout << "  [订阅者2] 收到 " << news.category << " 新闻: "
                  << news.headline << std::endl;
    });

    // 发布新闻
    std::cout << "\n--- 发布第一条新闻 ---" << std::endl;
    newsService.publish(NewsEvent{"科技", "AI大模型突破新纪录", "详细内容..."});

    // 取消订阅1
    newsService.unsubscribe(sub1);
    std::cout << "\n--- 订阅者1取消订阅后 ---" << std::endl;
    newsService.publish(NewsEvent{"财经", "股市大涨", "详细内容..."});

    // 多线程测试
    std::cout << "\n--- 多线程并发订阅/发布 ---" << std::endl;
    std::atomic<int> receivedCount{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&newsService, &receivedCount, i]() {
            auto conn = newsService.subscribe([&receivedCount, i](const NewsEvent&) {
                receivedCount++;
            });
            // 保持连接一小段时间
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            newsService.unsubscribe(conn);
        });
    }

    // 主线程发布事件
    for (int i = 0; i < 3; ++i) {
        newsService.publish(NewsEvent{"测试", "测试新闻", ""});
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "  多线程测试完成（无死锁、无崩溃即为通过）" << std::endl;
    std::cout << "  并发期间订阅者共收到 " << receivedCount.load()
              << " 次通知（具体次数取决于调度，不要求固定值）" << std::endl;
}

int main() {
    std::cout << "========== 观察者模式学习 Demo ==========" << std::endl;

    testPushVsPull();
    testSignalSlot();
    testWeakPtrLifecycle();
    testThreadSafeObserver();

    std::cout << "\n========== 观察者模式总结 ==========" << std::endl;
    std::cout << "核心思想：一对多依赖，状态变化自动通知所有依赖者" << std::endl;
    std::cout << "推模型：主题推送全部数据，简单但不够灵活" << std::endl;
    std::cout << "拉模型：主题只发通知，观察者按需拉取，更灵活" << std::endl;
    std::cout << "信号槽：现代C++风格，使用std::function，无需继承" << std::endl;
    std::cout << "weak_ptr：优雅解决观察者生命周期管理问题" << std::endl;
    std::cout << "线程安全：拷贝观察者列表后在锁外通知，避免死锁" << std::endl;
    std::cout << "设计原则：开闭原则、松耦合" << std::endl;

    return 0;
}
