/**
 * 适配器模式 (Adapter Pattern)
 *
 * 【模式意图】
 * 将一个类的接口转换成客户希望的另外一个接口。Adapter模式使得原本由于接口不兼容
 * 而不能一起工作的那些类可以一起工作。
 *
 * 【模式结构】
 * - Target: 目标接口，客户所期待的接口
 * - Adaptee: 需要适配的类
 * - Adapter: 适配器类，通过包装Adaptee将其接口转换成Target接口
 *
 * 分类：
 * - 类适配器：使用多重继承对一个接口与另一个接口进行匹配
 * - 对象适配器：使用对象组合的方式进行适配
 * - 接口适配器：使用抽象类提供默认实现，适配多方法接口
 *
 * 【适用场景】
 * 1. 想使用一个已经存在的类，而它的接口不符合需求
 * 2. 想创建一个可以复用的类，该类可以与其他不相关的类或不可预见的类协同工作
 * 3. 想使用一些已经存在的子类，但是不可能对每一个都进行子类化以匹配它们的接口
 *
 * 【优点】
 * 1. 将目标类和适配者类解耦，通过引入一个适配器类来重用现有的适配者类
 * 2. 增加了类的透明性和复用性，将具体的实现封装在适配者类中
 * 3. 灵活性和扩展性都非常好，符合开闭原则
 *
 * 【缺点】
 * 1. 过多使用适配器会让系统非常零乱
 * 2. 类适配器使用多重继承，有一定局限性
 *
 * 【现代C++实现方式】
 * 1. 对象适配器：使用组合（推荐），配合智能指针管理生命周期
 * 2. 类适配器：使用私有继承
 * 3. 函数适配器：使用std::function / lambda
 * 4. STL中的适配器：stack/queue/priority_queue 都是容器适配器
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <queue>
#include <functional>
#include <algorithm>
#include <cassert>

// ============================================================
// 目标接口（客户端期望的接口）
// ============================================================
class IMediaPlayer {
public:
    virtual ~IMediaPlayer() = default;
    virtual void play(const std::string& audioType, const std::string& fileName) = 0;
    virtual void stop() = 0;
    virtual bool isPlaying() const = 0;
};

// ============================================================
// 被适配者（Adaptee）- 不兼容的接口
// ============================================================

// 高级媒体播放器 - 接口与IMediaPlayer不同
class AdvancedMediaPlayer {
public:
    virtual ~AdvancedMediaPlayer() = default;

    // 接口不匹配：参数顺序不同，方法名不同
    virtual void playVideo(const std::string& filename, const std::string& format) {
        std::cout << "  [AdvancedMediaPlayer] 播放视频: " << filename
                  << " 格式: " << format << std::endl;
        isPlaying_ = true;
    }

    virtual void pause() {
        std::cout << "  [AdvancedMediaPlayer] 暂停播放" << std::endl;
        isPlaying_ = false;
    }

    virtual bool getStatus() const {
        return isPlaying_;
    }

protected:
    bool isPlaying_ = false;
};

// VLC播放器
class VlcPlayer : public AdvancedMediaPlayer {
public:
    void playVideo(const std::string& filename, const std::string& format) override {
        std::cout << "  [VlcPlayer] 使用VLC播放: " << filename
                  << " (" << format << ")" << std::endl;
        isPlaying_ = true;
    }
};

// MP4播放器
class Mp4Player : public AdvancedMediaPlayer {
public:
    void playVideo(const std::string& filename, const std::string& format) override {
        std::cout << "  [Mp4Player] 使用MP4解码器播放: " << filename
                  << " (" << format << ")" << std::endl;
        isPlaying_ = true;
    }
};

// ============================================================
// 1. 对象适配器（推荐方式：使用组合）
// ============================================================

/**
 * 对象适配器：持有被适配者的指针/引用，通过组合实现适配。
 * 优点：
 * - 一个适配器可以适配多个被适配者（如果需要）
 * - 符合"组合优于继承"的原则
 * - 更灵活，可在运行时切换被适配者
 */
class MediaAdapter_Object : public IMediaPlayer {
public:
    // 构造时指定被适配者
    explicit MediaAdapter_Object(std::unique_ptr<AdvancedMediaPlayer> adaptee)
        : adaptee_(std::move(adaptee)) {
    }

    // 适配play方法：参数顺序转换、方法名映射
    void play(const std::string& audioType, const std::string& fileName) override {
        // 接口转换：将 (type, filename) 转换为 (filename, format)
        std::cout << "[对象适配器] 转换调用: play(" << audioType << ", " << fileName
                  << ") -> playVideo(" << fileName << ", " << audioType << ")" << std::endl;
        adaptee_->playVideo(fileName, audioType);
    }

    // 适配stop方法：映射到pause
    void stop() override {
        std::cout << "[对象适配器] 转换调用: stop() -> pause()" << std::endl;
        adaptee_->pause();
    }

    // 适配isPlaying方法：映射到getStatus
    bool isPlaying() const override {
        return adaptee_->getStatus();
    }

    // 可以在运行时切换被适配者
    void setAdaptee(std::unique_ptr<AdvancedMediaPlayer> adaptee) {
        adaptee_ = std::move(adaptee);
    }

private:
    std::unique_ptr<AdvancedMediaPlayer> adaptee_;  // 组合被适配者
};

// ============================================================
// 2. 类适配器（使用私有继承）
// ============================================================

/**
 * 类适配器：使用私有继承方式，继承被适配者的实现。
 * 注意：C++支持多重继承，Java等语言不支持。
 *
 * 私有继承的语义是"根据...实现"，是一种组合的语法糖。
 * 但由于继承关系，适配器与被适配者的耦合度更高。
 */
class MediaAdapter_Class : public IMediaPlayer, private VlcPlayer {
public:
    void play(const std::string& audioType, const std::string& fileName) override {
        std::cout << "[类适配器] 转换调用: play(" << audioType << ", " << fileName
                  << ") -> playVideo(" << fileName << ", " << audioType << ")" << std::endl;
        // 调用父类（被适配者）的方法
        VlcPlayer::playVideo(fileName, audioType);
    }

    void stop() override {
        std::cout << "[类适配器] 转换调用: stop() -> pause()" << std::endl;
        VlcPlayer::pause();
    }

    bool isPlaying() const override {
        return VlcPlayer::getStatus();
    }
};

// ============================================================
// 3. 接口适配器（缺省适配器）
// ============================================================

/**
 * 接口适配器：当目标接口有很多方法，但客户端只需要其中一部分时，
 * 可以提供一个抽象类给出默认实现，客户端只需覆盖需要的方法。
 *
 * 这在C++中常用于：
 * - 事件监听器接口
 * - 回调接口
 * - 插件接口
 */

// 一个有很多方法的复杂接口
class IDataProcessor {
public:
    virtual ~IDataProcessor() = default;
    virtual void onStart() = 0;
    virtual void onData(const std::string& data) = 0;
    virtual void onProgress(int percent) = 0;
    virtual void onComplete() = 0;
    virtual void onError(const std::string& error) = 0;
    virtual void onCancel() = 0;
};

// 接口适配器：提供空的默认实现
class DataProcessorAdapter : public IDataProcessor {
public:
    void onStart() override {}          // 默认空实现
    void onData(const std::string&) override {}
    void onProgress(int) override {}
    void onComplete() override {}
    void onError(const std::string&) override {}
    void onCancel() override {}
};

// 客户端只需继承适配器，覆盖感兴趣的方法
class SimpleDataHandler : public DataProcessorAdapter {
public:
    void onData(const std::string& data) override {
        std::cout << "  [SimpleDataHandler] 收到数据: " << data << std::endl;
    }

    void onComplete() override {
        std::cout << "  [SimpleDataHandler] 数据处理完成" << std::endl;
    }
};

// ============================================================
// 4. STL中的适配器模式
// ============================================================

/**
 * STL中的容器适配器是适配器模式的经典例子：
 * - std::stack   -> 底层使用 deque/vector/list
 * - std::queue   -> 底层使用 deque/list
 * - std::priority_queue -> 底层使用 vector/deque
 *
 * 它们将底层容器的接口"适配"为栈/队列的接口。
 */
void demonstrateSTLAdapters() {
    std::cout << "\n--- STL容器适配器示例 ---" << std::endl;

    // stack适配器：将deque适配为LIFO结构
    std::stack<int> s;  // 底层默认使用std::deque
    s.push(1);
    s.push(2);
    s.push(3);
    std::cout << "  stack 顶部: " << s.top() << " (底层容器: deque)" << std::endl;

    // queue适配器：将deque适配为FIFO结构
    std::queue<std::string> q;
    q.push("first");
    q.push("second");
    std::cout << "  queue 前端: " << q.front() << " (底层容器: deque)" << std::endl;

    // priority_queue适配器：将vector适配为优先级队列
    std::priority_queue<int> pq;
    pq.push(30);
    pq.push(10);
    pq.push(20);
    std::cout << "  priority_queue 顶部: " << pq.top() << " (底层容器: vector)" << std::endl;
}

// ============================================================
// 5. 函数适配器（现代C++风格）
// ============================================================

/**
 * 使用std::function和lambda可以实现函数级别的适配。
 * 这是一种更轻量级的适配器，适用于函数接口不兼容的场景。
 */

// 旧的回调接口（签名不匹配）
using OldCallback = void(*)(int, const char*, void* userData);

// 新的回调接口
using NewCallback = std::function<void(const std::string& message, int code)>;

// 函数适配器：将旧风格回调适配为新风格
NewCallback adaptOldCallback(OldCallback oldFunc, void* userData) {
    return [oldFunc, userData](const std::string& message, int code) {
        oldFunc(code, message.c_str(), userData);
    };
}

// 旧风格的回调函数
void oldStyleCallback(int code, const char* msg, void* userData) {
    std::cout << "  [旧风格回调] code=" << code << ", msg=" << msg
              << ", userData=" << userData << std::endl;
}

// 使用新风格接口的函数
void processData(NewCallback callback) {
    callback("处理成功", 200);
}

// ============================================================
// 6. 双向适配器
// ============================================================

/**
 * 双向适配器：同时适配两个方向，使得两个不兼容的系统可以互相调用。
 * 这在集成两个独立开发的系统时非常有用。
 */

// 系统A的接口
class ISystemA {
public:
    virtual ~ISystemA() = default;
    virtual void sendMessageA(const std::string& msg) = 0;
    virtual std::string receiveMessageA() = 0;
};

// 系统B的接口
class ISystemB {
public:
    virtual ~ISystemB() = default;
    virtual void dispatchB(const std::string& payload) = 0;
    virtual std::string fetchB() = 0;
};

// 系统A的具体实现
class ConcreteSystemA : public ISystemA {
public:
    void sendMessageA(const std::string& msg) override {
        std::cout << "  [SystemA] 发送: " << msg << std::endl;
        lastSent_ = msg;
    }
    std::string receiveMessageA() override {
        return "SystemA的响应";
    }
private:
    std::string lastSent_;
};

// 双向适配器：同时实现两个接口，持有两个系统的引用
class TwoWayAdapter : public ISystemA, public ISystemB {
public:
    TwoWayAdapter(std::shared_ptr<ISystemA> systemA, std::shared_ptr<ISystemB> systemB)
        : systemA_(std::move(systemA)), systemB_(std::move(systemB)) {
    }

    // 适配SystemA的接口到SystemB
    void sendMessageA(const std::string& msg) override {
        std::cout << "  [双向适配器] SystemA.send -> SystemB.dispatch" << std::endl;
        systemB_->dispatchB(msg);
    }

    std::string receiveMessageA() override {
        std::cout << "  [双向适配器] SystemA.receive <- SystemB.fetch" << std::endl;
        return systemB_->fetchB();
    }

    // 适配SystemB的接口到SystemA
    void dispatchB(const std::string& payload) override {
        std::cout << "  [双向适配器] SystemB.dispatch -> SystemA.send" << std::endl;
        systemA_->sendMessageA(payload);
    }

    std::string fetchB() override {
        std::cout << "  [双向适配器] SystemB.fetch <- SystemA.receive" << std::endl;
        return systemA_->receiveMessageA();
    }

private:
    std::shared_ptr<ISystemA> systemA_;
    std::shared_ptr<ISystemB> systemB_;
};

// 系统B的具体实现（在适配器之后定义因为上面用到了）
class ConcreteSystemB : public ISystemB {
public:
    void dispatchB(const std::string& payload) override {
        std::cout << "  [SystemB] 收到调度: " << payload << std::endl;
    }
    std::string fetchB() override {
        return "SystemB的数据";
    }
};

// ============================================================
// 测试代码
// ============================================================

void testObjectAdapter() {
    std::cout << "\n========== 对象适配器测试 ==========" << std::endl;

    // 使用VLC播放器适配器
    std::cout << "\n--- 适配VlcPlayer ---" << std::endl;
    auto vlcAdapter = std::make_unique<MediaAdapter_Object>(
        std::make_unique<VlcPlayer>()
    );
    vlcAdapter->play("vlc", "movie.vlc");
    std::cout << "  播放状态: " << (vlcAdapter->isPlaying() ? "播放中" : "已停止") << std::endl;
    vlcAdapter->stop();
    std::cout << "  播放状态: " << (vlcAdapter->isPlaying() ? "播放中" : "已停止") << std::endl;

    // 运行时切换被适配者
    std::cout << "\n--- 运行时切换为Mp4Player ---" << std::endl;
    vlcAdapter->setAdaptee(std::make_unique<Mp4Player>());
    vlcAdapter->play("mp4", "video.mp4");
    std::cout << "  播放状态: " << (vlcAdapter->isPlaying() ? "播放中" : "已停止") << std::endl;
}

void testClassAdapter() {
    std::cout << "\n========== 类适配器测试 ==========" << std::endl;

    MediaAdapter_Class adapter;
    adapter.play("vlc", "movie.vlc");
    std::cout << "  播放状态: " << (adapter.isPlaying() ? "播放中" : "已停止") << std::endl;
    adapter.stop();
    std::cout << "  播放状态: " << (adapter.isPlaying() ? "播放中" : "已停止") << std::endl;
}

void testInterfaceAdapter() {
    std::cout << "\n========== 接口适配器测试 ==========" << std::endl;

    // 客户端只需关注感兴趣的回调
    SimpleDataHandler handler;

    // 模拟数据处理流程
    std::cout << "  (模拟数据处理流程)" << std::endl;
    handler.onStart();      // 默认空实现，无输出
    handler.onData("Hello");
    handler.onProgress(50); // 默认空实现，无输出
    handler.onData("World");
    handler.onProgress(100); // 默认空实现，无输出
    handler.onComplete();
}

void testFunctionAdapter() {
    std::cout << "\n========== 函数适配器测试 ==========" << std::endl;

    int userData = 42;
    auto adapted = adaptOldCallback(oldStyleCallback, &userData);
    processData(adapted);

    // 直接使用lambda适配
    std::cout << "\n  --- 使用lambda直接适配 ---" << std::endl;
    processData([](const std::string& msg, int code) {
        std::cout << "  [Lambda适配] code=" << code << ", msg=" << msg << std::endl;
    });
}

void testTwoWayAdapter() {
    std::cout << "\n========== 双向适配器测试 ==========" << std::endl;

    auto sysA = std::make_shared<ConcreteSystemA>();
    auto sysB = std::make_shared<ConcreteSystemB>();

    auto adapter = std::make_shared<TwoWayAdapter>(sysA, sysB);

    // 通过适配器，SystemA可以调用SystemB
    std::cout << "\n--- SystemA 通过适配器调用 SystemB ---" << std::endl;
    adapter->sendMessageA("来自A的消息");
    std::string response = adapter->receiveMessageA();
    std::cout << "  SystemA收到: " << response << std::endl;

    // 反过来，SystemB也可以调用SystemA
    std::cout << "\n--- SystemB 通过适配器调用 SystemA ---" << std::endl;
    adapter->dispatchB("来自B的调度");
    std::string data = adapter->fetchB();
    std::cout << "  SystemB收到: " << data << std::endl;
}

int main() {
    std::cout << "========== 适配器模式学习 Demo ==========" << std::endl;

    testObjectAdapter();
    testClassAdapter();
    testInterfaceAdapter();
    demonstrateSTLAdapters();
    testFunctionAdapter();
    testTwoWayAdapter();

    std::cout << "\n========== 适配器模式总结 ==========" << std::endl;
    std::cout << "核心思想：将不兼容的接口转换为客户期望的接口" << std::endl;
    std::cout << "对象适配器：使用组合，灵活可扩展（推荐）" << std::endl;
    std::cout << "类适配器：使用继承，耦合度较高" << std::endl;
    std::cout << "接口适配器：为多方法接口提供默认实现" << std::endl;
    std::cout << "函数适配器：使用std::function/lambda适配函数签名" << std::endl;
    std::cout << "STL实例：stack/queue/priority_queue都是容器适配器" << std::endl;
    std::cout << "设计原则：组合优于继承，开闭原则" << std::endl;

    return 0;
}
