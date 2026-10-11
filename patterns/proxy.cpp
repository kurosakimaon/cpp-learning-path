/**
 * 代理模式 (Proxy Pattern)
 *
 * 【模式意图】
 * 为其他对象提供一种代理以控制对这个对象的访问。
 *
 * 【模式结构】
 * - Subject: 抽象主题，定义真实主题和代理的共同接口
 * - RealSubject: 真实主题，定义代理所代表的真实对象
 * - Proxy: 代理，保存一个指向真实主题的引用，控制对真实主题的访问
 *
 * 代理的种类：
 * 1. 远程代理(Remote Proxy)：为一个对象在不同的地址空间提供局部代表
 * 2. 虚拟代理(Virtual Proxy)：根据需要创建开销很大的对象（延迟加载）
 * 3. 保护代理(Protection Proxy)：控制对原始对象的访问，用于对象有不同的访问权限
 * 4. 智能指针(Smart Pointer)：代替裸指针，添加额外功能（引用计数、自动释放）
 * 5. 缓存代理(Cache Proxy)：为开销大的操作结果提供缓存
 *
 * 【适用场景】
 * 1. 需要为一个对象提供不同级别的访问控制
 * 2. 需要创建开销很大的对象时，延迟到真正使用时才创建
 * 3. 需要访问远程对象时
 * 4. 需要为访问添加额外逻辑（日志、计数、缓存等）
 *
 * 【优点】
 * 1. 代理模式在客户端与目标对象之间起到一个中介和保护目标对象的作用
 * 2. 可以扩展目标对象的功能
 * 3. 能将客户端与目标对象分离，降低系统耦合度
 * 4. 虚拟代理可以优化系统性能（延迟加载）
 *
 * 【缺点】
 * 1. 增加了代理对象，可能导致请求处理速度变慢
 * 2. 实现代理模式需要额外的工作，增加系统复杂度
 *
 * 【现代C++实现方式】
 * 1. 虚拟代理：延迟加载大对象
 * 2. 保护代理：权限控制
 * 3. 智能指针：shared_ptr/unique_ptr是代理模式的典型应用
 * 4. 异步代理：封装异步操作
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>
#include <sstream>

// ============================================================
// 抽象主题：图像接口
// ============================================================
class IImage {
public:
    virtual ~IImage() = default;
    virtual void display() = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual std::string getFilename() const = 0;
};

// ============================================================
// 真实主题：真实图像（加载开销大）
// ============================================================
class RealImage : public IImage {
public:
    explicit RealImage(const std::string& filename)
        : filename_(filename) {
        loadFromDisk();
    }

    void display() override {
        std::cout << "  [RealImage] 显示图像: " << filename_
                  << " (" << width_ << "x" << height_ << ")" << std::endl;
    }

    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }
    std::string getFilename() const override { return filename_; }

private:
    void loadFromDisk() {
        std::cout << "  [RealImage] 从磁盘加载图像: " << filename_
                  << " (模拟耗时操作...)" << std::endl;
        // 模拟加载耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // 模拟从文件中读取的图像信息
        width_ = 1920;
        height_ = 1080;
    }

    std::string filename_;
    int width_ = 0;
    int height_ = 0;
};

// ============================================================
// 1. 虚拟代理（延迟加载）
// ============================================================

/**
 * 虚拟代理：在真正需要图像数据时才加载。
 * 对于图像列表、文档预览等场景非常有用，可以显著提升启动速度。
 */
class VirtualImageProxy : public IImage {
public:
    explicit VirtualImageProxy(const std::string& filename)
        : filename_(filename) {
        std::cout << "  [VirtualImageProxy] 创建代理: " << filename_
                  << " (未加载实际图像)" << std::endl;
    }

    void display() override {
        // 延迟加载：第一次显示时才创建真实对象
        if (!realImage_) {
            std::cout << "  [VirtualImageProxy] 延迟加载中..." << std::endl;
            realImage_ = std::make_unique<RealImage>(filename_);
        }
        realImage_->display();
    }

    int getWidth() const override {
        if (realImage_) return realImage_->getWidth();
        return 0; // 未加载时返回0或默认值
    }

    int getHeight() const override {
        if (realImage_) return realImage_->getHeight();
        return 0;
    }

    std::string getFilename() const override {
        return filename_;
    }

    // 检查是否已加载
    bool isLoaded() const { return realImage_ != nullptr; }

private:
    std::string filename_;
    mutable std::unique_ptr<RealImage> realImage_;  // 延迟创建的真实对象
};

// ============================================================
// 2. 保护代理（权限控制）
// ============================================================

// 抽象主题：文档
class IDocument {
public:
    virtual ~IDocument() = default;
    virtual std::string getContent() const = 0;
    virtual void setContent(const std::string& content) = 0;
    virtual std::string getMetadata() const = 0;
};

// 真实主题：机密文档
class ConfidentialDocument : public IDocument {
public:
    explicit ConfidentialDocument(std::string content)
        : content_(std::move(content)) {
    }

    std::string getContent() const override { return content_; }
    void setContent(const std::string& content) override { content_ = content; }
    std::string getMetadata() const override {
        return "机密文档 | 大小: " + std::to_string(content_.size()) + " 字节";
    }

private:
    std::string content_;
};

// 用户角色
enum class UserRole {
    Guest,      // 访客 - 只能看元数据
    Viewer,     // 查看者 - 可以看内容
    Editor,     // 编辑者 - 可以修改内容
    Admin       // 管理员 - 完全控制
};

/**
 * 保护代理：根据用户角色控制对文档的访问权限。
 * 这是一种常见的安全代理实现。
 */
class DocumentProtectionProxy : public IDocument {
public:
    DocumentProtectionProxy(std::unique_ptr<ConfidentialDocument> doc, UserRole role)
        : realDoc_(std::move(doc)), role_(role) {
    }

    std::string getContent() const override {
        if (role_ == UserRole::Guest) {
            return "[权限不足] 访客无法查看文档内容";
        }
        return realDoc_->getContent();
    }

    void setContent(const std::string& content) override {
        if (role_ == UserRole::Guest || role_ == UserRole::Viewer) {
            std::cout << "  [保护代理] 权限不足，无法修改文档" << std::endl;
            return;
        }
        std::cout << "  [保护代理] 允许修改文档" << std::endl;
        realDoc_->setContent(content);
    }

    std::string getMetadata() const override {
        // 所有角色都可以查看元数据
        return realDoc_->getMetadata();
    }

private:
    std::unique_ptr<ConfidentialDocument> realDoc_;
    UserRole role_;
};

// ============================================================
// 3. 缓存代理
// ============================================================

/**
 * 缓存代理：为开销大的计算结果提供缓存。
 * 当相同的请求再次发生时，直接返回缓存的结果。
 */
class IDataService {
public:
    virtual ~IDataService() = default;
    virtual std::string queryData(const std::string& key) = 0;
    virtual int computeValue(int input) = 0;
};

class RealDataService : public IDataService {
public:
    std::string queryData(const std::string& key) override {
        std::cout << "  [RealDataService] 从数据库查询: " << key << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 模拟查询耗时
        return "数据_" + key + "_" + std::to_string(rand() % 1000);
    }

    int computeValue(int input) override {
        std::cout << "  [RealDataService] 执行复杂计算: " << input << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30)); // 模拟计算耗时
        return input * input + 42;
    }
};

class CachedDataServiceProxy : public IDataService {
public:
    explicit CachedDataServiceProxy(std::unique_ptr<IDataService> realService)
        : realService_(std::move(realService)) {
    }

    std::string queryData(const std::string& key) override {
        // 查询缓存
        auto it = dataCache_.find(key);
        if (it != dataCache_.end()) {
            std::cout << "  [缓存代理] 命中缓存: " << key << std::endl;
            return it->second;
        }

        // 缓存未命中，调用真实服务
        std::string result = realService_->queryData(key);
        dataCache_[key] = result;
        return result;
    }

    int computeValue(int input) override {
        auto it = computeCache_.find(input);
        if (it != computeCache_.end()) {
            std::cout << "  [缓存代理] 命中计算缓存: " << input << std::endl;
            return it->second;
        }

        int result = realService_->computeValue(input);
        computeCache_[input] = result;
        return result;
    }

    // 清除缓存
    void invalidateCache() {
        dataCache_.clear();
        computeCache_.clear();
        std::cout << "  [缓存代理] 缓存已清除" << std::endl;
    }

    size_t getCacheSize() const {
        return dataCache_.size() + computeCache_.size();
    }

private:
    std::unique_ptr<IDataService> realService_;
    std::map<std::string, std::string> dataCache_;
    std::map<int, int> computeCache_;
};

// ============================================================
// 4. 智能指针作为代理模式
// ============================================================

/**
 * C++标准库中的智能指针（shared_ptr、unique_ptr、weak_ptr）就是代理模式的典型应用。
 * 它们代理了裸指针的行为，并添加了额外的功能：
 * - unique_ptr: 独占所有权，自动释放
 * - shared_ptr: 引用计数，共享所有权
 * - weak_ptr: 弱引用，不增加引用计数，解决循环引用
 *
 * 下面实现一个简化的shared_ptr来展示代理模式的思想。
 */

// 简化的引用计数控制块
struct ControlBlock {
    int refCount = 1;
    int weakCount = 0;
};

// 简化版 shared_ptr
template <typename T>
class SimpleSharedPtr {
public:
    SimpleSharedPtr() : ptr_(nullptr), control_(nullptr) {}

    explicit SimpleSharedPtr(T* ptr)
        : ptr_(ptr), control_(ptr ? new ControlBlock() : nullptr) {
    }

    // 拷贝构造
    SimpleSharedPtr(const SimpleSharedPtr& other)
        : ptr_(other.ptr_), control_(other.control_) {
        if (control_) {
            ++control_->refCount;
        }
    }

    // 移动构造
    SimpleSharedPtr(SimpleSharedPtr&& other) noexcept
        : ptr_(other.ptr_), control_(other.control_) {
        other.ptr_ = nullptr;
        other.control_ = nullptr;
    }

    // 析构
    ~SimpleSharedPtr() {
        release();
    }

    // 拷贝赋值
    SimpleSharedPtr& operator=(const SimpleSharedPtr& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            control_ = other.control_;
            if (control_) {
                ++control_->refCount;
            }
        }
        return *this;
    }

    // 移动赋值
    SimpleSharedPtr& operator=(SimpleSharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            control_ = other.control_;
            other.ptr_ = nullptr;
            other.control_ = nullptr;
        }
        return *this;
    }

    // 指针运算符 -> 代理访问
    T* operator->() const {
        return ptr_;
    }

    // 解引用运算符 * 代理访问
    T& operator*() const {
        return *ptr_;
    }

    T* get() const { return ptr_; }
    int use_count() const { return control_ ? control_->refCount : 0; }
    explicit operator bool() const { return ptr_ != nullptr; }

private:
    void release() {
        if (control_ && --control_->refCount == 0) {
            delete ptr_;
            delete control_;
        }
        ptr_ = nullptr;
        control_ = nullptr;
    }

    T* ptr_;
    ControlBlock* control_;
};

// 测试类
class TestResource {
public:
    explicit TestResource(int id) : id_(id) {
        std::cout << "  [TestResource] 资源 " << id_ << " 被创建" << std::endl;
    }
    ~TestResource() {
        std::cout << "  [TestResource] 资源 " << id_ << " 被释放" << std::endl;
    }
    void doSomething() {
        std::cout << "  [TestResource] 资源 " << id_ << " 执行操作" << std::endl;
    }
private:
    int id_;
};

// ============================================================
// 5. 日志/计数代理
// ============================================================

/**
 * 日志代理：在访问真实对象前后添加日志记录。
 * 这是AOP（面向切面编程）思想的一种体现。
 */
class ICounter {
public:
    virtual ~ICounter() = default;
    virtual void increment() = 0;
    virtual int getValue() const = 0;
};

class RealCounter : public ICounter {
public:
    void increment() override { value_++; }
    int getValue() const override { return value_; }
private:
    int value_ = 0;
};

class LoggingCounterProxy : public ICounter {
public:
    explicit LoggingCounterProxy(std::unique_ptr<ICounter> realCounter)
        : realCounter_(std::move(realCounter)) {
    }

    void increment() override {
        int oldValue = realCounter_->getValue();
        realCounter_->increment();
        int newValue = realCounter_->getValue();
        accessCount_++;
        std::cout << "  [日志代理] increment: " << oldValue << " -> " << newValue
                  << " (总访问次数: " << accessCount_ << ")" << std::endl;
    }

    int getValue() const override {
        accessCount_++;
        int value = realCounter_->getValue();
        std::cout << "  [日志代理] getValue = " << value
                  << " (总访问次数: " << accessCount_ << ")" << std::endl;
        return value;
    }

    int getAccessCount() const { return accessCount_; }

private:
    std::unique_ptr<ICounter> realCounter_;
    mutable int accessCount_ = 0;
};

// ============================================================
// 测试代码
// ============================================================

void testVirtualProxy() {
    std::cout << "\n========== 虚拟代理（延迟加载）测试 ==========" << std::endl;

    // 创建图像列表 - 注意：此时并未实际加载图像数据
    std::cout << "\n--- 创建图像列表（不加载实际数据） ---" << std::endl;
    std::vector<std::unique_ptr<IImage>> images;
    images.push_back(std::make_unique<VirtualImageProxy>("photo1.jpg"));
    images.push_back(std::make_unique<VirtualImageProxy>("photo2.jpg"));
    images.push_back(std::make_unique<VirtualImageProxy>("photo3.jpg"));

    std::cout << "\n  (图像列表创建完成，共 " << images.size()
              << " 张，但均未加载实际数据)" << std::endl;

    // 只显示第一张图像 - 只有第一张会被加载
    std::cout << "\n--- 只显示第一张图像 ---" << std::endl;
    images[0]->display();

    // 检查哪些被加载了
    std::cout << "\n--- 检查加载状态 ---" << std::endl;
    for (size_t i = 0; i < images.size(); ++i) {
        auto* proxy = dynamic_cast<VirtualImageProxy*>(images[i].get());
        std::cout << "  图像 " << (i+1) << " (" << images[i]->getFilename()
                  << "): " << (proxy->isLoaded() ? "已加载" : "未加载") << std::endl;
    }

    // 显示第二张
    std::cout << "\n--- 显示第二张图像 ---" << std::endl;
    images[1]->display();
}

void testProtectionProxy() {
    std::cout << "\n========== 保护代理（权限控制）测试 ==========" << std::endl;

    auto createDoc = []() {
        return std::make_unique<ConfidentialDocument>("这是机密文件内容：公司年度财务报表...");
    };

    // 访客角色
    std::cout << "\n--- 访客 (Guest) ---" << std::endl;
    DocumentProtectionProxy guestDoc(createDoc(), UserRole::Guest);
    std::cout << "  元数据: " << guestDoc.getMetadata() << std::endl;
    std::cout << "  内容: " << guestDoc.getContent() << std::endl;
    guestDoc.setContent("修改内容");

    // 查看者角色
    std::cout << "\n--- 查看者 (Viewer) ---" << std::endl;
    DocumentProtectionProxy viewerDoc(createDoc(), UserRole::Viewer);
    std::cout << "  元数据: " << viewerDoc.getMetadata() << std::endl;
    std::cout << "  内容: " << viewerDoc.getContent() << std::endl;
    viewerDoc.setContent("修改内容");

    // 编辑者角色
    std::cout << "\n--- 编辑者 (Editor) ---" << std::endl;
    DocumentProtectionProxy editorDoc(createDoc(), UserRole::Editor);
    std::cout << "  元数据: " << editorDoc.getMetadata() << std::endl;
    std::cout << "  原始内容: " << editorDoc.getContent() << std::endl;
    editorDoc.setContent("更新后的机密内容");
    std::cout << "  更新后内容: " << editorDoc.getContent() << std::endl;
}

void testCacheProxy() {
    std::cout << "\n========== 缓存代理测试 ==========" << std::endl;

    auto proxy = std::make_unique<CachedDataServiceProxy>(
        std::make_unique<RealDataService>()
    );

    // 第一次查询（缓存未命中）
    std::cout << "\n--- 第一次查询 ---" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::string r1 = proxy->queryData("user_123");
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "  结果: " << r1 << " | 耗时: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms" << std::endl;

    // 第二次查询相同的key（缓存命中）
    std::cout << "\n--- 第二次查询（相同key） ---" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::string r2 = proxy->queryData("user_123");
    end = std::chrono::high_resolution_clock::now();
    std::cout << "  结果: " << r2 << " | 耗时: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms" << std::endl;

    // 计算缓存
    std::cout << "\n--- 计算缓存测试 ---" << std::endl;
    proxy->computeValue(10);
    proxy->computeValue(10);
    proxy->computeValue(20);
    proxy->computeValue(10);

    std::cout << "\n  缓存条目数: " << proxy->getCacheSize() << std::endl;
}

void testSmartPointerProxy() {
    std::cout << "\n========== 智能指针（代理模式）测试 ==========" << std::endl;

    std::cout << "\n--- SimpleSharedPtr 基本用法 ---" << std::endl;
    {
        SimpleSharedPtr<TestResource> ptr1(new TestResource(1));
        std::cout << "  ptr1 引用计数: " << ptr1.use_count() << std::endl;

        SimpleSharedPtr<TestResource> ptr2 = ptr1;
        std::cout << "  拷贝后 ptr1 引用计数: " << ptr1.use_count() << std::endl;
        std::cout << "  ptr2 引用计数: " << ptr2.use_count() << std::endl;

        // 通过代理访问真实对象
        ptr1->doSomething();
        (*ptr2).doSomething();
    }
    std::cout << "  (离开作用域，资源自动释放)" << std::endl;

    // 对比标准库的shared_ptr
    std::cout << "\n--- std::shared_ptr (标准库智能指针) ---" << std::endl;
    {
        auto sp1 = std::make_shared<TestResource>(2);
        std::cout << "  sp1 引用计数: " << sp1.use_count() << std::endl;

        auto sp2 = sp1;
        std::cout << "  拷贝后 sp1 引用计数: " << sp1.use_count() << std::endl;

        sp1->doSomething();
    }
    std::cout << "  (离开作用域，资源自动释放)" << std::endl;
}

void testLoggingProxy() {
    std::cout << "\n========== 日志代理测试 ==========" << std::endl;

    auto counter = std::make_unique<LoggingCounterProxy>(
        std::make_unique<RealCounter>()
    );

    counter->increment();
    counter->increment();
    counter->getValue();
    counter->increment();
    counter->getValue();

    std::cout << "  最终值: " << counter->getValue() << std::endl;
    std::cout << "  总访问次数: " << counter->getAccessCount() << std::endl;
}

int main() {
    std::cout << "========== 代理模式学习 Demo ==========" << std::endl;

    testVirtualProxy();
    testProtectionProxy();
    testCacheProxy();
    testSmartPointerProxy();
    testLoggingProxy();

    std::cout << "\n========== 代理模式总结 ==========" << std::endl;
    std::cout << "核心思想：为对象提供一个代理，控制对该对象的访问" << std::endl;
    std::cout << "虚拟代理：延迟加载大对象，提升启动性能" << std::endl;
    std::cout << "保护代理：控制访问权限，实现安全控制" << std::endl;
    std::cout << "缓存代理：缓存结果，避免重复计算/查询" << std::endl;
    std::cout << "智能指针：代理裸指针，管理内存生命周期" << std::endl;
    std::cout << "日志代理：添加访问日志，实现AOP效果" << std::endl;
    std::cout << "与装饰器的区别：代理控制访问，装饰器添加功能" << std::endl;
    std::cout << "设计原则：单一职责、开闭原则" << std::endl;

    return 0;
}
