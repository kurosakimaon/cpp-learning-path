/**
 * 单例模式 (Singleton Pattern)
 *
 * 【模式意图】
 * 确保一个类只有一个实例，并提供一个全局访问点来访问该实例。
 *
 * 【模式结构】
 * - Singleton: 单例类，负责创建自己的唯一实例，同时提供一个访问该实例的全局方法
 *
 * 【适用场景】
 * 1. 系统只需要一个实例对象，例如配置管理器、连接池、线程池
 * 2. 客户端调用单个实例时不允许直接实例化（构造函数私有）
 * 3. 资源共享场景，避免多个实例导致的资源冲突
 *
 * 【优点】
 * 1. 提供了对唯一实例的受控访问
 * 2. 避免重复创建对象，节省系统资源
 * 3. 全局唯一访问点，便于统一管理
 *
 * 【缺点】
 * 1. 违反单一职责原则（既负责业务逻辑，又负责实例管理）
 * 2. 难以扩展（没有接口，不能继承）
 * 3. 测试困难（难以Mock，全局状态难以隔离）
 * 4. 多线程环境下需要考虑线程安全
 * 5. 隐藏了类之间的依赖关系，使得代码耦合度高
 *
 * 【现代C++实现方式】
 * 本文件展示了单例模式的多种实现方式及其演进：
 * 1. 饿汉式 - 程序启动时即创建实例
 * 2. 懒汉式（非线程安全） - 首次使用时创建
 * 3. 线程安全版本（Double-Checked Locking） - 双重检查锁定
 * 4. Meyers' Singleton - 利用局部静态变量（C++11起线程安全）
 * 5. 依赖注入的单例替代方案 - 更现代的设计思路
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <cassert>

// ============================================================
// 1. 饿汉式单例 (Eager Initialization)
// ============================================================
/**
 * 特点：程序启动时就创建实例，天生线程安全
 * 优点：实现简单，线程安全
 * 缺点：如果实例从未使用，会浪费资源；不支持延迟初始化；构造函数抛异常时处理困难
 */
class EagerSingleton {
public:
    // 禁止拷贝和移动
    EagerSingleton(const EagerSingleton&) = delete;
    EagerSingleton& operator=(const EagerSingleton&) = delete;
    EagerSingleton(EagerSingleton&&) = delete;
    EagerSingleton& operator=(EagerSingleton&&) = delete;

    // 全局访问点
    static EagerSingleton& getInstance() {
        return instance_;
    }

    void doSomething() const {
        std::cout << "[EagerSingleton] 执行操作, data = " << data_ << std::endl;
    }

    void setData(const std::string& data) {
        data_ = data;
    }

private:
    // 私有构造函数
    EagerSingleton() : data_("default") {
        std::cout << "[EagerSingleton] 构造函数被调用（饿汉式：程序启动即创建）" << std::endl;
    }

    ~EagerSingleton() {
        std::cout << "[EagerSingleton] 析构函数被调用" << std::endl;
    }

    std::string data_;

    // 静态实例：在程序进入main之前就已初始化
    static EagerSingleton instance_;
};

// 静态成员初始化（类外定义）
EagerSingleton EagerSingleton::instance_;


// ============================================================
// 2. 懒汉式单例（非线程安全）
// ============================================================
/**
 * 特点：首次使用时才创建实例，实现懒加载
 * 优点：延迟初始化，节省资源
 * 缺点：多线程环境下不安全，可能创建多个实例
 */
class LazySingleton {
public:
    LazySingleton(const LazySingleton&) = delete;
    LazySingleton& operator=(const LazySingleton&) = delete;

    // 非线程安全版本
    static LazySingleton* getInstance() {
        if (instance_ == nullptr) {
            // 多线程下，多个线程可能同时通过上面的检查，导致创建多个实例
            instance_ = new LazySingleton();
        }
        return instance_;
    }

    void doSomething() const {
        std::cout << "[LazySingleton] 执行操作, id = " << id_ << std::endl;
    }

private:
    LazySingleton() : id_(++count_) {
        std::cout << "[LazySingleton] 构造函数被调用, id = " << id_ << std::endl;
    }

    ~LazySingleton() = default;

    int id_;
    static int count_;
    static LazySingleton* instance_;
};

LazySingleton* LazySingleton::instance_ = nullptr;
int LazySingleton::count_ = 0;


// ============================================================
// 3. 线程安全单例 - Double-Checked Locking (DCL)
// ============================================================
/**
 * 特点：双重检查锁定，既保证线程安全又尽量减少锁的开销
 *
 * 注意：在C++11之前，DCL由于指令重排问题可能失效。
 * C++11及以后，使用std::atomic和memory_order_acquire/memory_order_release
 * 可以正确实现DCL。
 */
class ThreadSafeSingleton {
public:
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;

    static ThreadSafeSingleton* getInstance() {
        // 第一次检查：无锁，快速路径
        ThreadSafeSingleton* tmp = instance_.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            // 加锁，只有第一次创建时才会进入
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：防止多个线程同时通过第一次检查
            tmp = instance_.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new ThreadSafeSingleton();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }

    void doSomething() const {
        std::cout << "[ThreadSafeSingleton] 执行操作, value = " << value_ << std::endl;
    }

    void setValue(int v) {
        value_ = v;
    }

private:
    ThreadSafeSingleton() : value_(0) {
        std::cout << "[ThreadSafeSingleton] 构造函数被调用（DCL方式）" << std::endl;
    }

    ~ThreadSafeSingleton() = default;

    int value_;
    static std::atomic<ThreadSafeSingleton*> instance_;
    static std::mutex mutex_;
};

std::atomic<ThreadSafeSingleton*> ThreadSafeSingleton::instance_{nullptr};
std::mutex ThreadSafeSingleton::mutex_;


// ============================================================
// 4. Meyers' Singleton (最推荐的C++单例实现)
// ============================================================
/**
 * 特点：利用函数内的局部静态变量实现
 *
 * C++11标准规定：局部静态变量的初始化是线程安全的。
 * 当多个线程同时进入该变量的初始化语句时，只有一个线程进行初始化，
 * 其他线程等待初始化完成。这就是所谓的"Magic Statics"。
 *
 * 这是最简洁、最安全、最高效的单例实现方式。
 */
class MeyersSingleton {
public:
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
    MeyersSingleton(MeyersSingleton&&) = delete;
    MeyersSingleton& operator=(MeyersSingleton&&) = delete;

    static MeyersSingleton& getInstance() {
        // C++11及以后：局部静态变量的初始化是线程安全的
        static MeyersSingleton instance;
        return instance;
    }

    void doSomething() const {
        std::cout << "[MeyersSingleton] 执行操作, counter = " << counter_ << std::endl;
    }

    void increment() {
        ++counter_;
    }

private:
    MeyersSingleton() : counter_(0) {
        std::cout << "[MeyersSingleton] 构造函数被调用（Meyers方式，线程安全）" << std::endl;
    }

    ~MeyersSingleton() {
        std::cout << "[MeyersSingleton] 析构函数被调用" << std::endl;
    }

    int counter_;
};


// ============================================================
// 5. 依赖注入的单例替代方案
// ============================================================
/**
 * 单例模式的一个主要问题是它引入了全局状态，使得代码难以测试和解耦。
 *
 * 更现代的做法是使用依赖注入(Dependency Injection)：
 * - 将"只有一个实例"的约束交给应用层保证，而不是类本身
 * - 类只负责自己的业务逻辑，不负责实例管理
 * - 这样的类更容易测试、更容易扩展
 *
 * 这本质上是一种"单一职责原则"的体现。
 */

// 业务接口
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
};

// 具体实现 - 控制台日志
class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "[ConsoleLogger] " << message << std::endl;
    }
};

// 具体实现 - 文件日志（示例，实际会写入文件）
class FileLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "[FileLogger] 写入文件: " << message << std::endl;
    }
};

// 使用依赖注入的服务类
class UserService {
public:
    // 通过构造函数注入依赖，而不是在内部创建单例
    explicit UserService(std::shared_ptr<ILogger> logger)
        : logger_(std::move(logger)) {
    }

    void createUser(const std::string& username) {
        logger_->log("创建用户: " + username);
        // ... 实际业务逻辑
    }

private:
    std::shared_ptr<ILogger> logger_;
};

// 在应用启动时组装对象（组合根），保证只有一个实例
class AppContext {
public:
    static AppContext& instance() {
        static AppContext ctx;
        return ctx;
    }

    std::shared_ptr<ILogger> getLogger() const {
        return logger_;
    }

    std::shared_ptr<UserService> getUserService() const {
        return userService_;
    }

private:
    AppContext() {
        logger_ = std::make_shared<ConsoleLogger>();
        userService_ = std::make_shared<UserService>(logger_);
    }

    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<UserService> userService_;
};


// ============================================================
// 测试代码
// ============================================================

// 测试多线程下的单例
void testThreadSafety() {
    std::cout << "\n=== 测试多线程单例 ===" << std::endl;

    constexpr int numThreads = 10;
    std::vector<std::thread> threads;

    // 测试Meyers单例的线程安全性
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i]() {
            // 每个线程都获取单例并操作
            auto& inst = MeyersSingleton::getInstance();
            inst.increment();
            std::cout << "  线程 " << i << " 获取到实例地址: " << &inst << std::endl;
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "最终 counter = " << MeyersSingleton::getInstance().counter_ << std::endl;
    // 注意：counter_在这里可以访问是因为我们在测试中故意将其暴露
    // 实际使用中应该通过接口访问
}

// 测试依赖注入方案
void testDependencyInjection() {
    std::cout << "\n=== 测试依赖注入的单例替代方案 ===" << std::endl;

    // 使用应用上下文获取服务
    auto userService = AppContext::instance().getUserService();
    userService->createUser("Alice");
    userService->createUser("Bob");

    // 测试时可以轻松替换为Mock
    class MockLogger : public ILogger {
    public:
        void log(const std::string& message) override {
            lastMessage = message;
            ++logCount;
        }
        std::string lastMessage;
        int logCount = 0;
    };

    auto mockLogger = std::make_shared<MockLogger>();
    UserService testService(mockLogger);
    testService.createUser("TestUser");

    assert(mockLogger->logCount == 1);
    assert(mockLogger->lastMessage.find("TestUser") != std::string::npos);
    std::cout << "  Mock测试通过: logCount = " << mockLogger->logCount
              << ", lastMessage = " << mockLogger->lastMessage << std::endl;
}

int main() {
    std::cout << "========== 单例模式学习 Demo ==========" << std::endl;

    // 注意：饿汉式单例在进入main之前就已经创建了
    std::cout << "\n=== 1. 饿汉式单例 ===" << std::endl;
    EagerSingleton::getInstance().doSomething();
    EagerSingleton::getInstance().setData("Hello Singleton");
    EagerSingleton::getInstance().doSomething();

    // 验证只有一个实例
    std::cout << "  两次获取的实例地址相同: "
              << (&EagerSingleton::getInstance() == &EagerSingleton::getInstance() ? "是" : "否")
              << std::endl;

    std::cout << "\n=== 2. 懒汉式单例（非线程安全） ===" << std::endl;
    LazySingleton::getInstance()->doSomething();
    LazySingleton::getInstance()->doSomething();
    std::cout << "  count_ = " << LazySingleton::count_
              << " (如果等于1说明只有一个实例)" << std::endl;

    std::cout << "\n=== 3. 线程安全单例（DCL） ===" << std::endl;
    ThreadSafeSingleton::getInstance()->doSomething();
    ThreadSafeSingleton::getInstance()->setValue(42);
    ThreadSafeSingleton::getInstance()->doSomething();

    std::cout << "\n=== 4. Meyers' Singleton（最推荐） ===" << std::endl;
    MeyersSingleton::getInstance().doSomething();
    MeyersSingleton::getInstance().increment();
    MeyersSingleton::getInstance().increment();
    MeyersSingleton::getInstance().doSomething();

    // 多线程测试
    testThreadSafety();

    // 依赖注入方案
    testDependencyInjection();

    std::cout << "\n========== 单例模式总结 ==========" << std::endl;
    std::cout << "推荐实现方式: Meyers' Singleton (局部静态变量)" << std::endl;
    std::cout << "更优设计思路: 依赖注入 + 应用层保证唯一实例" << std::endl;
    std::cout << "单例的本质问题: 全局状态、难以测试、隐藏依赖" << std::endl;

    return 0;
}
