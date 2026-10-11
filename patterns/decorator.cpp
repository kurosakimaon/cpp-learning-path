/**
 * 装饰器模式 (Decorator Pattern)
 *
 * 【模式意图】
 * 动态地给一个对象添加一些额外的职责。就增加功能来说，Decorator模式相比生成子类
 * 更为灵活。
 *
 * 【模式结构】
 * - Component: 抽象构件，定义一个抽象接口，可以给这些对象动态地添加职责
 * - ConcreteComponent: 具体构件，定义一个具体的对象，也可以给这个对象添加一些职责
 * - Decorator: 抽象装饰类，持有一个Component对象的引用，并定义一个与Component接口一致的接口
 * - ConcreteDecorator: 具体装饰类，向组件添加职责
 *
 * 【适用场景】
 * 1. 在不影响其他对象的情况下，以动态、透明的方式给单个对象添加职责
 * 2. 处理那些可以撤销的职责
 * 3. 当不能采用生成子类的方法进行扩充时
 *    - 可能有大量独立的扩展，为支持每一种组合将产生大量的子类
 *    - 类定义被隐藏或不能用于生成子类
 *
 * 【优点】
 * 1. 比静态继承更灵活，可以动态地添加和撤销功能
 * 2. 避免在层次结构高层的类有太多的特征
 * 3. 可以用多个具体装饰类装饰一个对象，创造出很多不同行为的组合
 * 4. 装饰类和被装饰类可以独立发展，不会相互耦合
 *
 * 【缺点】
 * 1. 会产生很多小对象，增加系统复杂度
 * 2. 排错困难，需要层层排查
 * 3. 比继承更加灵活，但也更加容易出错
 *
 * 【现代C++实现方式】
 * 1. 经典装饰器：使用组合+继承
 * 2. 流插入运算符风格：模仿iostream的<<操作符链式装饰
 * 3. 模板装饰器：编译期装饰，零运行时开销
 * 4. std::function装饰器：函数式装饰
 */

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <cassert>

// ============================================================
// 基础接口：数据流
// ============================================================
class IStream {
public:
    virtual ~IStream() = default;
    virtual std::string read() = 0;
    virtual void write(const std::string& data) = 0;
    virtual std::string getDescription() const = 0;
};

// 具体构件：基础文件流
class FileStream : public IStream {
public:
    explicit FileStream(const std::string& filename)
        : filename_(filename) {
    }

    std::string read() override {
        std::cout << "  [FileStream] 从文件 " << filename_ << " 读取数据" << std::endl;
        return "file_content_data";
    }

    void write(const std::string& data) override {
        std::cout << "  [FileStream] 向文件 " << filename_ << " 写入数据: " << data << std::endl;
    }

    std::string getDescription() const override {
        return "文件流 (" + filename_ + ")";
    }

private:
    std::string filename_;
};

// 具体构件：内存流
class MemoryStream : public IStream {
public:
    std::string read() override {
        std::cout << "  [MemoryStream] 从内存读取数据" << std::endl;
        return buffer_;
    }

    void write(const std::string& data) override {
        std::cout << "  [MemoryStream] 向内存写入数据: " << data << std::endl;
        buffer_ += data;
    }

    std::string getDescription() const override {
        return "内存流";
    }

private:
    std::string buffer_;
};

// ============================================================
// 抽象装饰器基类
// ============================================================

/**
 * 装饰器基类：持有一个被装饰对象的引用，并实现相同的接口。
 * 所有具体装饰器都继承自这个类。
 */
class StreamDecorator : public IStream {
public:
    explicit StreamDecorator(std::unique_ptr<IStream> stream)
        : stream_(std::move(stream)) {
    }

    // 默认实现：委托给被装饰对象
    std::string read() override {
        return stream_->read();
    }

    void write(const std::string& data) override {
        stream_->write(data);
    }

    std::string getDescription() const override {
        return stream_->getDescription();
    }

protected:
    std::unique_ptr<IStream> stream_;  // 被装饰的对象
};

// ============================================================
// 具体装饰器1：加密装饰
// ============================================================
class EncryptedStream : public StreamDecorator {
public:
    EncryptedStream(std::unique_ptr<IStream> stream, std::string key)
        : StreamDecorator(std::move(stream)), key_(std::move(key)) {
    }

    std::string read() override {
        std::cout << "  [EncryptedStream] 解密数据 (key=" << key_ << ")" << std::endl;
        std::string data = StreamDecorator::read();
        return decrypt(data);
    }

    void write(const std::string& data) override {
        std::cout << "  [EncryptedStream] 加密数据 (key=" << key_ << ")" << std::endl;
        std::string encrypted = encrypt(data);
        StreamDecorator::write(encrypted);
    }

    std::string getDescription() const override {
        return "加密的 " + StreamDecorator::getDescription();
    }

private:
    std::string encrypt(const std::string& data) {
        // 简单的异或加密示例
        std::string result = data;
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] ^= key_[i % key_.size()];
        }
        return "[ENCRYPTED]" + result;
    }

    std::string decrypt(const std::string& data) {
        // 简化：直接返回原文
        return data + " (已解密)";
    }

    std::string key_;
};

// ============================================================
// 具体装饰器2：压缩装饰
// ============================================================
class CompressedStream : public StreamDecorator {
public:
    explicit CompressedStream(std::unique_ptr<IStream> stream)
        : StreamDecorator(std::move(stream)) {
    }

    std::string read() override {
        std::string data = StreamDecorator::read();
        std::cout << "  [CompressedStream] 解压数据" << std::endl;
        return decompress(data);
    }

    void write(const std::string& data) override {
        std::cout << "  [CompressedStream] 压缩数据" << std::endl;
        std::string compressed = compress(data);
        StreamDecorator::write(compressed);
    }

    std::string getDescription() const override {
        return "压缩的 " + StreamDecorator::getDescription();
    }

private:
    std::string compress(const std::string& data) {
        return "[COMPRESSED:" + std::to_string(data.size()) + "bytes]";
    }

    std::string decompress(const std::string& data) {
        return data + " (已解压)";
    }
};

// ============================================================
// 具体装饰器3：缓冲装饰
// ============================================================
class BufferedStream : public StreamDecorator {
public:
    BufferedStream(std::unique_ptr<IStream> stream, size_t bufferSize)
        : StreamDecorator(std::move(stream)), bufferSize_(bufferSize) {
    }

    std::string read() override {
        if (readBuffer_.empty()) {
            std::cout << "  [BufferedStream] 读取缓冲区 (大小=" << bufferSize_ << ")" << std::endl;
            readBuffer_ = StreamDecorator::read();
        } else {
            std::cout << "  [BufferedStream] 从缓冲区读取" << std::endl;
        }
        return readBuffer_;
    }

    void write(const std::string& data) override {
        writeBuffer_ += data;
        if (writeBuffer_.size() >= bufferSize_) {
            flush();
        } else {
            std::cout << "  [BufferedStream] 写入缓冲区 (当前大小="
                      << writeBuffer_.size() << "/" << bufferSize_ << ")" << std::endl;
        }
    }

    void flush() {
        if (!writeBuffer_.empty()) {
            std::cout << "  [BufferedStream] 刷新缓冲区" << std::endl;
            StreamDecorator::write(writeBuffer_);
            writeBuffer_.clear();
        }
    }

    std::string getDescription() const override {
        return "缓冲的 " + StreamDecorator::getDescription();
    }

private:
    size_t bufferSize_;
    std::string readBuffer_;
    std::string writeBuffer_;
};

// ============================================================
// 4. 流插入运算符风格装饰（模仿iostream）
// ============================================================

/**
 * 模仿C++ iostream的<<操作符风格，使用链式调用装饰对象。
 * 这种方式更加符合C++的使用习惯。
 */

// 基础日志接口
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
    virtual std::string getName() const = 0;
};

// 具体日志：控制台
class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "  [ConsoleLogger] " << message << std::endl;
    }
    std::string getName() const override { return "ConsoleLogger"; }
};

// 装饰器基类
class LoggerDecorator : public ILogger {
public:
    explicit LoggerDecorator(std::shared_ptr<ILogger> logger)
        : logger_(std::move(logger)) {}

    void log(const std::string& message) override {
        logger_->log(message);
    }

    std::string getName() const override {
        return logger_->getName();
    }

protected:
    std::shared_ptr<ILogger> logger_;
};

// 时间戳装饰
class TimestampLogger : public LoggerDecorator {
public:
    using LoggerDecorator::LoggerDecorator;

    void log(const std::string& message) override {
        std::string timestamped = "[2024-01-15 10:30:45] " + message;
        LoggerDecorator::log(timestamped);
    }

    std::string getName() const override {
        return "Timestamped(" + LoggerDecorator::getName() + ")";
    }
};

// 日志级别装饰
class LevelLogger : public LoggerDecorator {
public:
    LevelLogger(std::shared_ptr<ILogger> logger, const std::string& level)
        : LoggerDecorator(std::move(logger)), level_(level) {}

    void log(const std::string& message) override {
        std::string leveled = "[" + level_ + "] " + message;
        LoggerDecorator::log(leveled);
    }

    std::string getName() const override {
        return "Level(" + LoggerDecorator::getName() + ")";
    }

private:
    std::string level_;
};

// 辅助函数：创建装饰器链（流式风格）
std::shared_ptr<ILogger> operator<<(std::shared_ptr<ILogger> logger,
                                    std::function<std::shared_ptr<ILogger>(std::shared_ptr<ILogger>)> decorator) {
    return decorator(logger);
}

// 装饰器工厂函数
auto withTimestamp = [](std::shared_ptr<ILogger> logger) {
    return std::make_shared<TimestampLogger>(std::move(logger));
};

auto withLevel(const std::string& level) {
    return [level](std::shared_ptr<ILogger> logger) {
        return std::make_shared<LevelLogger>(std::move(logger), level);
    };
}

// ============================================================
// 5. 模板装饰器（编译期装饰）
// ============================================================

/**
 * 使用模板实现编译期装饰，零运行时开销。
 * 装饰器直接包装在类型系统中，不需要虚函数调用。
 *
 * 这是现代C++中非常强大的技术，在性能敏感的场景中非常有用。
 */

// 基础组件接口（使用CRTP模式）
template <typename Derived>
class ICoffee {
public:
    std::string getDescription() const {
        return static_cast<const Derived*>(this)->descriptionImpl();
    }

    double cost() const {
        return static_cast<const Derived*>(this)->costImpl();
    }
};

// 具体咖啡：基础款
class SimpleCoffee : public ICoffee<SimpleCoffee> {
public:
    std::string descriptionImpl() const {
        return "简单咖啡";
    }

    double costImpl() const {
        return 10.0;
    }
};

// 模板装饰器：加牛奶
template <typename CoffeeT>
class MilkDecorator : public ICoffee<MilkDecorator<CoffeeT>> {
public:
    explicit MilkDecorator(CoffeeT coffee, int shots = 1)
        : coffee_(std::move(coffee)), shots_(shots) {}

    std::string descriptionImpl() const {
        return coffee_.getDescription() + " + 牛奶(" + std::to_string(shots_) + "份)";
    }

    double costImpl() const {
        return coffee_.cost() + 2.0 * shots_;
    }

private:
    CoffeeT coffee_;
    int shots_;
};

// 模板装饰器：加糖
template <typename CoffeeT>
class SugarDecorator : public ICoffee<SugarDecorator<CoffeeT>> {
public:
    explicit SugarDecorator(CoffeeT coffee, int spoons = 1)
        : coffee_(std::move(coffee)), spoons_(spoons) {}

    std::string descriptionImpl() const {
        return coffee_.getDescription() + " + 糖(" + std::to_string(spoons_) + "勺)";
    }

    double costImpl() const {
        return coffee_.cost() + 0.5 * spoons_;
    }

private:
    CoffeeT coffee_;
    int spoons_;
};

// 模板装饰器：加奶泡
template <typename CoffeeT>
class FoamDecorator : public ICoffee<FoamDecorator<CoffeeT>> {
public:
    explicit FoamDecorator(CoffeeT coffee)
        : coffee_(std::move(coffee)) {}

    std::string descriptionImpl() const {
        return coffee_.getDescription() + " + 奶泡";
    }

    double costImpl() const {
        return coffee_.cost() + 3.0;
    }

private:
    CoffeeT coffee_;
};

// 辅助函数：方便创建装饰器
template <typename CoffeeT>
auto withMilk(CoffeeT coffee, int shots = 1) {
    return MilkDecorator<CoffeeT>(std::move(coffee), shots);
}

template <typename CoffeeT>
auto withSugar(CoffeeT coffee, int spoons = 1) {
    return SugarDecorator<CoffeeT>(std::move(coffee), spoons);
}

template <typename CoffeeT>
auto withFoam(CoffeeT coffee) {
    return FoamDecorator<CoffeeT>(std::move(coffee));
}

// ============================================================
// 6. 函数装饰器（函数式编程风格）
// ============================================================

/**
 * 使用std::function和高阶函数实现函数级别的装饰。
 * 这种方式非常轻量，适合装饰函数/回调。
 */

// 日志装饰器：给函数添加日志
template <typename Func>
auto logDecorator(Func func, const std::string& name) {
    return [func, name](auto&&... args) {
        std::cout << "  [LogDecorator] 进入 " << name << std::endl;
        auto result = func(std::forward<decltype(args)>(args)...);
        std::cout << "  [LogDecorator] 退出 " << name << std::endl;
        return result;
    };
}

// 计时装饰器：给函数添加性能计时
template <typename Func>
auto timingDecorator(Func func, const std::string& name) {
    return [func, name](auto&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func(std::forward<decltype(args)>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "  [TimingDecorator] " << name << " 耗时: "
                  << duration.count() << " 微秒" << std::endl;
        return result;
    };
}

// 重试装饰器：给函数添加重试逻辑
template <typename Func>
auto retryDecorator(Func func, int maxRetries) {
    return [func, maxRetries](auto&&... args) {
        for (int i = 0; i < maxRetries; ++i) {
            try {
                return func(std::forward<decltype(args)>(args)...);
            } catch (const std::exception& e) {
                std::cout << "  [RetryDecorator] 第 " << (i + 1)
                          << " 次尝试失败: " << e.what() << std::endl;
                if (i == maxRetries - 1) throw;
            }
        }
        return func(std::forward<decltype(args)>(args)...);
    };
}

// ============================================================
// 测试代码
// ============================================================

void testClassicDecorator() {
    std::cout << "\n========== 经典装饰器模式测试 ==========" << std::endl;

    // 基础文件流
    std::cout << "\n--- 基础文件流 ---" << std::endl;
    auto basicFile = std::make_unique<FileStream>("data.txt");
    std::cout << "  描述: " << basicFile->getDescription() << std::endl;
    basicFile->write("Hello World");
    basicFile->read();

    // 加密装饰
    std::cout << "\n--- 加密的文件流 ---" << std::endl;
    auto encryptedFile = std::make_unique<EncryptedStream>(
        std::make_unique<FileStream>("secret.txt"),
        "my_secret_key"
    );
    std::cout << "  描述: " << encryptedFile->getDescription() << std::endl;
    encryptedFile->write("敏感数据");
    encryptedFile->read();

    // 多重装饰：加密 + 压缩 + 缓冲
    std::cout << "\n--- 加密+压缩+缓冲的内存流 ---" << std::endl;
    auto superStream = std::make_unique<EncryptedStream>(
        std::make_unique<CompressedStream>(
            std::make_unique<BufferedStream>(
                std::make_unique<MemoryStream>(),
                1024
            )
        ),
        "key123"
    );
    std::cout << "  描述: " << superStream->getDescription() << std::endl;
    superStream->write("重要数据");
    superStream->read();
}

void testStreamStyleDecorator() {
    std::cout << "\n========== 流式风格装饰器测试 ==========" << std::endl;

    // 使用<<操作符风格链式装饰
    auto logger = std::make_shared<ConsoleLogger>()
        << withTimestamp
        << withLevel("INFO");

    std::cout << "  日志器名称: " << logger->getName() << std::endl;
    logger->log("用户登录成功");
    logger->log("数据处理完成");

    // 不同的装饰组合
    std::cout << "\n--- 仅添加时间戳 ---" << std::endl;
    auto logger2 = std::make_shared<ConsoleLogger>() << withTimestamp;
    logger2->log("简单日志消息");

    std::cout << "\n--- 仅添加日志级别 ---" << std::endl;
    auto logger3 = std::make_shared<ConsoleLogger>() << withLevel("ERROR");
    logger3->log("发生错误");
}

void testTemplateDecorator() {
    std::cout << "\n========== 模板装饰器测试（编译期装饰） ==========" << std::endl;

    // 简单咖啡
    SimpleCoffee simple;
    std::cout << "\n--- " << simple.getDescription() << " ---" << std::endl;
    std::cout << "  价格: ¥" << simple.cost() << std::endl;

    // 加牛奶的咖啡
    auto milkCoffee = withMilk(SimpleCoffee{});
    std::cout << "\n--- " << milkCoffee.getDescription() << " ---" << std::endl;
    std::cout << "  价格: ¥" << milkCoffee.cost() << std::endl;

    // 拿铁：咖啡 + 牛奶 + 奶泡 + 糖
    auto latte = withFoam(withSugar(withMilk(SimpleCoffee{}, 2)));
    std::cout << "\n--- " << latte.getDescription() << " ---" << std::endl;
    std::cout << "  价格: ¥" << latte.cost() << std::endl;

    // 焦糖玛奇朵
    auto caramel = withFoam(withSugar(withMilk(SimpleCoffee{}, 3), 2));
    std::cout << "\n--- " << caramel.getDescription() << " ---" << std::endl;
    std::cout << "  价格: ¥" << caramel.cost() << std::endl;

    std::cout << "\n  (模板装饰器在编译期确定，零运行时虚函数开销)" << std::endl;
}

void testFunctionDecorator() {
    std::cout << "\n========== 函数装饰器测试 ==========" << std::endl;

    // 原始函数
    auto compute = [](int a, int b) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return a + b;
    };

    // 单个装饰
    std::cout << "\n--- 日志装饰 ---" << std::endl;
    auto loggedCompute = logDecorator(compute, "compute");
    int result = loggedCompute(3, 4);
    std::cout << "  结果: " << result << std::endl;

    // 多个装饰组合
    std::cout << "\n--- 日志 + 计时装饰 ---" << std::endl;
    auto timedLoggedCompute = timingDecorator(
        logDecorator(compute, "compute"),
        "compute"
    );
    result = timedLoggedCompute(10, 20);
    std::cout << "  结果: " << result << std::endl;

    // 重试装饰
    std::cout << "\n--- 重试装饰 ---" << std::endl;
    int callCount = 0;
    auto flakyFunction = [&callCount](int x) -> int {
        callCount++;
        if (callCount < 3) {
            throw std::runtime_error("临时故障");
        }
        return x * 2;
    };

    auto retryFunction = retryDecorator(flakyFunction, 3);
    try {
        int res = retryFunction(21);
        std::cout << "  最终结果: " << res << " (尝试次数: " << callCount << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  最终失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "========== 装饰器模式学习 Demo ==========" << std::endl;

    testClassicDecorator();
    testStreamStyleDecorator();
    testTemplateDecorator();
    testFunctionDecorator();

    std::cout << "\n========== 装饰器模式总结 ==========" << std::endl;
    std::cout << "核心思想：动态地给对象添加额外职责，比继承更灵活" << std::endl;
    std::cout << "经典装饰器：组合+继承，运行时动态装饰" << std::endl;
    std::cout << "流式风格：模仿iostream的<<操作符，链式调用" << std::endl;
    std::cout << "模板装饰器：编译期装饰，零运行时开销（性能最优）" << std::endl;
    std::cout << "函数装饰器：函数式风格，装饰函数/回调" << std::endl;
    std::cout << "与继承对比：装饰器可以灵活组合，避免类爆炸" << std::endl;
    std::cout << "设计原则：开闭原则、组合优于继承" << std::endl;

    return 0;
}
