/**
 * 建造者模式 (Builder Pattern)
 *
 * 【模式意图】
 * 将一个复杂对象的构建与它的表示分离，使得同样的构建过程可以创建不同的表示。
 *
 * 【模式结构】
 * - Builder: 抽象建造者接口，定义创建产品各部件的操作
 * - ConcreteBuilder: 具体建造者，实现Builder接口，构造和装配各个部件
 * - Product: 产品角色，被构建的复杂对象
 * - Director: 指挥者，构建一个使用Builder接口的对象
 *
 * 【适用场景】
 * 1. 创建复杂对象的算法应该独立于该对象的组成部分以及它们的装配方式
 * 2. 构造过程必须允许被构造的对象有不同的表示
 * 3. 对象内部结构复杂，有多个可选参数
 * 4. 构造过程需要按特定顺序执行
 *
 * 【优点】
 * 1. 将复杂对象的创建过程封装起来，客户端不需要知道对象内部的组成细节
 * 2. 可以逐步构建对象，构建过程可控
 * 3. 相同的构建过程可以创建不同的产品表示
 * 4. 符合单一职责原则，将复杂的构建逻辑从业务逻辑中分离
 *
 * 【缺点】
 * 1. 会产生额外的Builder类，增加代码复杂度
 * 2. 如果产品内部变化频繁，需要频繁修改Builder类
 *
 * 【现代C++实现方式】
 * 1. 链式调用 / Fluent Interface - 流式API设计
 * 2. 模板建造者 - 编译期构建复杂对象
 * 3. 与lambda结合 - 灵活的构建步骤
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <functional>

// ============================================================
// 产品类：计算机（复杂对象）
// ============================================================
class Computer {
public:
    void setCPU(const std::string& cpu) { cpu_ = cpu; }
    void setGPU(const std::string& gpu) { gpu_ = gpu; }
    void setRAM(int gb) { ramGB_ = gb; }
    void setStorage(int gb) { storageGB_ = gb; }
    void setMotherboard(const std::string& mb) { motherboard_ = mb; }
    void setPowerSupply(const std::string& ps) { powerSupply_ = ps; }
    void addPeripheral(const std::string& peripheral) {
        peripherals_.push_back(peripheral);
    }

    std::string getSpecs() const {
        std::ostringstream oss;
        oss << "计算机配置:\n"
            << "  CPU: " << cpu_ << "\n"
            << "  GPU: " << gpu_ << "\n"
            << "  内存: " << ramGB_ << " GB\n"
            << "  存储: " << storageGB_ << " GB\n"
            << "  主板: " << motherboard_ << "\n"
            << "  电源: " << powerSupply_ << "\n"
            << "  外设: ";
        for (size_t i = 0; i < peripherals_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << peripherals_[i];
        }
        oss << "\n  总价: ¥" << calculatePrice();
        return oss.str();
    }

    int calculatePrice() const {
        int price = 0;
        // 简化的价格计算
        if (cpu_.find("i9") != std::string::npos) price += 4500;
        else if (cpu_.find("i7") != std::string::npos) price += 3000;
        else if (cpu_.find("i5") != std::string::npos) price += 1500;
        else price += 800;

        if (gpu_.find("4090") != std::string::npos) price += 15000;
        else if (gpu_.find("4070") != std::string::npos) price += 5000;
        else if (gpu_.find("集成") != std::string::npos) price += 0;
        else price += 2000;

        price += ramGB_ * 30;      // 每GB 30元
        price += storageGB_ * 1;   // 每GB 1元
        price += 1000;             // 主板
        price += 600;              // 电源
        price += static_cast<int>(peripherals_.size()) * 200;
        return price;
    }

private:
    std::string cpu_;
    std::string gpu_;
    int ramGB_ = 0;
    int storageGB_ = 0;
    std::string motherboard_;
    std::string powerSupply_;
    std::vector<std::string> peripherals_;
};

// ============================================================
// 传统建造者模式
// ============================================================

// 抽象建造者
class IComputerBuilder {
public:
    virtual ~IComputerBuilder() = default;
    virtual void buildCPU() = 0;
    virtual void buildGPU() = 0;
    virtual void buildRAM() = 0;
    virtual void buildStorage() = 0;
    virtual void buildMotherboard() = 0;
    virtual void buildPowerSupply() = 0;
    virtual void buildPeripherals() = 0;
    virtual std::unique_ptr<Computer> getResult() = 0;
};

// 具体建造者：高端游戏PC
class GamingComputerBuilder : public IComputerBuilder {
public:
    GamingComputerBuilder() {
        computer_ = std::make_unique<Computer>();
    }

    void buildCPU() override { computer_->setCPU("Intel i9-13900K"); }
    void buildGPU() override { computer_->setGPU("NVIDIA RTX 4090"); }
    void buildRAM() override { computer_->setRAM(64); }
    void buildStorage() override { computer_->setStorage(2000); }
    void buildMotherboard() override { computer_->setMotherboard("ASUS ROG MAXIMUS"); }
    void buildPowerSupply() override { computer_->setPowerSupply("海盗船 1200W 金牌"); }
    void buildPeripherals() override {
        computer_->addPeripheral("机械键盘");
        computer_->addPeripheral("电竞鼠标");
        computer_->addPeripheral("4K显示器");
    }

    std::unique_ptr<Computer> getResult() override {
        return std::move(computer_);
    }

private:
    std::unique_ptr<Computer> computer_;
};

// 具体建造者：办公电脑
class OfficeComputerBuilder : public IComputerBuilder {
public:
    OfficeComputerBuilder() {
        computer_ = std::make_unique<Computer>();
    }

    void buildCPU() override { computer_->setCPU("Intel i5-13400"); }
    void buildGPU() override { computer_->setGPU("集成显卡"); }
    void buildRAM() override { computer_->setRAM(16); }
    void buildStorage() override { computer_->setStorage(512); }
    void buildMotherboard() override { computer_->setMotherboard("技嘉 B760"); }
    void buildPowerSupply() override { computer_->setPowerSupply("航嘉 500W"); }
    void buildPeripherals() override {
        computer_->addPeripheral("普通键盘");
        computer_->addPeripheral("普通鼠标");
    }

    std::unique_ptr<Computer> getResult() override {
        return std::move(computer_);
    }

private:
    std::unique_ptr<Computer> computer_;
};

// 指挥者：控制构建顺序
class ComputerDirector {
public:
    void setBuilder(std::unique_ptr<IComputerBuilder> builder) {
        builder_ = std::move(builder);
    }

    std::unique_ptr<Computer> build() {
        // 按照标准顺序构建
        builder_->buildMotherboard();
        builder_->buildCPU();
        builder_->buildGPU();
        builder_->buildRAM();
        builder_->buildStorage();
        builder_->buildPowerSupply();
        builder_->buildPeripherals();
        return builder_->getResult();
    }

private:
    std::unique_ptr<IComputerBuilder> builder_;
};

// ============================================================
// Fluent Builder（流式建造者）- 现代C++风格
// ============================================================

/**
 * 流式建造者通过链式调用提供更优雅的API。
 * 每个setter方法返回*this，使得调用可以链接在一起。
 * 这是现代C++中最常用的建造者实现方式。
 */
class FluentComputerBuilder {
public:
    FluentComputerBuilder() : computer_(std::make_unique<Computer>()) {}

    // 每个方法返回*this，支持链式调用
    FluentComputerBuilder& withCPU(const std::string& cpu) {
        computer_->setCPU(cpu);
        return *this;
    }

    FluentComputerBuilder& withGPU(const std::string& gpu) {
        computer_->setGPU(gpu);
        return *this;
    }

    FluentComputerBuilder& withRAM(int gb) {
        computer_->setRAM(gb);
        return *this;
    }

    FluentComputerBuilder& withStorage(int gb) {
        computer_->setStorage(gb);
        return *this;
    }

    FluentComputerBuilder& withMotherboard(const std::string& mb) {
        computer_->setMotherboard(mb);
        return *this;
    }

    FluentComputerBuilder& withPowerSupply(const std::string& ps) {
        computer_->setPowerSupply(ps);
        return *this;
    }

    FluentComputerBuilder& addPeripheral(const std::string& peripheral) {
        computer_->addPeripheral(peripheral);
        return *this;
    }

    // 构建完成，返回产品
    std::unique_ptr<Computer> build() {
        validate();
        return std::move(computer_);
    }

private:
    // 构建前验证
    void validate() const {
        // 简单的验证逻辑
        if (computer_->calculatePrice() == 1600) { // 只有基础配件
            // 不抛出异常，仅示例
        }
    }

    std::unique_ptr<Computer> computer_;
};

// ============================================================
// 模板建造者 - 编译期构建
// ============================================================

/**
 * 利用C++模板，可以在编译期构建对象。
 * 这对于配置对象、策略对象等场景非常有用。
 *
 * 模板建造者的优势：
 * 1. 零运行时开销
 * 2. 编译期验证配置的正确性
 * 3. 可以根据配置生成不同的代码
 */

// 编译期配置标记
struct HasCPU {};
struct HasGPU {};
struct HasRAM {};

// 模板建造者，使用模板参数跟踪已配置的选项
template <typename... Configs>
class TemplateComputerBuilder {
public:
    // 每个with方法返回一个新的建造者类型，添加对应的配置标记
    auto withCPU(const std::string& cpu) {
        cpu_ = cpu;
        return TemplateComputerBuilder<Configs..., HasCPU>{cpu_, gpu_, ram_};
    }

    auto withGPU(const std::string& gpu) {
        gpu_ = gpu;
        return TemplateComputerBuilder<Configs..., HasGPU>{cpu_, gpu_, ram_};
    }

    auto withRAM(int gb) {
        ram_ = gb;
        return TemplateComputerBuilder<Configs..., HasRAM>{cpu_, gpu_, ram_};
    }

    // build方法只能在所有必需配置都设置后才能调用
    // 这里使用SFINAE（替换失败不是错误）来实现编译期检查
    template <typename T = void>
    std::unique_ptr<Computer> build(
        typename std::enable_if<
            (std::is_same<Configs, HasCPU>::value || ...) &&
            (std::is_same<Configs, HasRAM>::value || ...),
            T
        >::type* = 0
    ) {
        auto computer = std::make_unique<Computer>();
        computer->setCPU(cpu_);
        computer->setGPU(gpu_.empty() ? "集成显卡" : gpu_);
        computer->setRAM(ram_);
        computer->setStorage(256); // 默认值
        computer->setMotherboard("标准主板");
        computer->setPowerSupply("标准电源");
        return computer;
    }

private:
    // 允许不同模板参数的建造者访问私有成员
    template <typename...>
    friend class TemplateComputerBuilder;

    TemplateComputerBuilder(const std::string& cpu, const std::string& gpu, int ram)
        : cpu_(cpu), gpu_(gpu), ram_(ram) {}

    std::string cpu_;
    std::string gpu_;
    int ram_ = 0;
};

// 空参数的初始建造者
template <>
class TemplateComputerBuilder<> {
public:
    TemplateComputerBuilder() = default;

    auto withCPU(const std::string& cpu) {
        return TemplateComputerBuilder<HasCPU>{}.withCPU(cpu);
    }

    auto withGPU(const std::string& gpu) {
        return TemplateComputerBuilder<HasGPU>{}.withGPU(gpu);
    }

    auto withRAM(int gb) {
        return TemplateComputerBuilder<HasRAM>{}.withRAM(gb);
    }

    // 缺少配置时调用build会编译失败
};


// ============================================================
// 测试代码
// ============================================================

void testClassicBuilder() {
    std::cout << "\n========== 传统建造者模式测试 ==========" << std::endl;

    ComputerDirector director;

    // 构建游戏电脑
    std::cout << "\n--- 构建游戏电脑 ---" << std::endl;
    director.setBuilder(std::make_unique<GamingComputerBuilder>());
    auto gamingPC = director.build();
    std::cout << gamingPC->getSpecs() << std::endl;

    // 构建办公电脑
    std::cout << "\n--- 构建办公电脑 ---" << std::endl;
    director.setBuilder(std::make_unique<OfficeComputerBuilder>());
    auto officePC = director.build();
    std::cout << officePC->getSpecs() << std::endl;
}

void testFluentBuilder() {
    std::cout << "\n========== 流式建造者(Fluent Interface)测试 ==========" << std::endl;

    // 链式调用，灵活配置
    std::cout << "\n--- 自定义高端工作站 ---" << std::endl;
    auto workstation = FluentComputerBuilder()
        .withCPU("AMD Ryzen 9 7950X")
        .withGPU("NVIDIA RTX 4070 Ti")
        .withRAM(128)
        .withStorage(4000)
        .withMotherboard("华硕 ProArt X670E")
        .withPowerSupply("海韵 1000W 钛金")
        .addPeripheral("专业显示器")
        .addPeripheral("绘图板")
        .build();
    std::cout << workstation->getSpecs() << std::endl;

    // 更简单的配置
    std::cout << "\n--- 简单家用电脑 ---" << std::endl;
    auto homePC = FluentComputerBuilder()
        .withCPU("Intel i3-13100")
        .withRAM(8)
        .withStorage(256)
        .withMotherboard("华擎 H610")
        .withPowerSupply("长城 300W")
        .build();
    std::cout << homePC->getSpecs() << std::endl;
}

void testTemplateBuilder() {
    std::cout << "\n========== 模板建造者测试 ==========" << std::endl;

    // 正确配置：有CPU和内存
    std::cout << "\n--- 完整配置的电脑 ---" << std::endl;
    auto pc1 = TemplateComputerBuilder<>()
        .withCPU("Intel i7-13700K")
        .withGPU("NVIDIA RTX 4070")
        .withRAM(32)
        .build();
    std::cout << pc1->getSpecs() << std::endl;

    // 只有CPU和内存，没有GPU（使用默认值）
    std::cout << "\n--- 无独立显卡的电脑 ---" << std::endl;
    auto pc2 = TemplateComputerBuilder<>()
        .withCPU("AMD Ryzen 5 7600")
        .withRAM(16)
        .build();
    std::cout << pc2->getSpecs() << std::endl;

    // 以下代码会编译失败（缺少必要配置），取消注释试试
    /*
    auto pc3 = TemplateComputerBuilder<>()
        .withCPU("Intel i5")
        .build();  // 错误：缺少RAM配置
    */

    std::cout << "  (模板建造者在编译期验证配置完整性)" << std::endl;
}

int main() {
    std::cout << "========== 建造者模式学习 Demo ==========" << std::endl;

    testClassicBuilder();
    testFluentBuilder();
    testTemplateBuilder();

    std::cout << "\n========== 建造者模式总结 ==========" << std::endl;
    std::cout << "核心思想：将复杂对象的构建过程与表示分离" << std::endl;
    std::cout << "传统建造者：Director控制构建顺序，Builder实现具体构建" << std::endl;
    std::cout << "流式建造者：链式调用API，灵活简洁，现代C++最常用" << std::endl;
    std::cout << "模板建造者：编译期构建，零运行时开销，可做编译期验证" << std::endl;
    std::cout << "适用场景：对象构造复杂、参数多、需要按步骤构建" << std::endl;

    return 0;
}
