/**
 * 工厂方法模式 + 抽象工厂模式 (Factory Method + Abstract Factory)
 *
 * 【模式意图】
 * 工厂方法：定义一个创建产品对象的工厂接口，让子类决定实例化哪一个类，将对象创建延迟到子类。
 * 抽象工厂：提供一个创建一系列相关或相互依赖对象的接口，而无需指定它们具体的类。
 *
 * 【模式结构】
 * 工厂方法：
 * - Product: 产品接口
 * - ConcreteProduct: 具体产品
 * - Factory: 工厂接口（声明工厂方法）
 * - ConcreteFactory: 具体工厂（实现工厂方法，创建具体产品）
 *
 * 抽象工厂：
 * - AbstractFactory: 抽象工厂接口（声明一组创建产品的方法）
 * - ConcreteFactory: 具体工厂（实现创建产品族的方法）
 * - AbstractProductX: 抽象产品接口
 * - ConcreteProductX: 具体产品
 * - Client: 客户端，只使用抽象接口
 *
 * 【适用场景】
 * 工厂方法：
 * 1. 类不知道它所必须创建的对象的类
 * 2. 类希望由它的子类来指定它所创建的对象
 * 3. 类将创建对象的职责委托给多个帮助子类中的某一个
 *
 * 抽象工厂：
 * 1. 系统要独立于它的产品的创建、组合和表示时
 * 2. 系统要由多个产品系列中的一个来配置时
 * 3. 要强调一系列相关的产品对象的设计以便进行联合使用时
 * 4. 提供一个产品类库，而只想显示它们的接口而不是实现时
 *
 * 【优点】
 * 1. 将产品创建与使用分离，符合开闭原则
 * 2. 抽象工厂保证了产品族的一致性
 * 3. 具体产品类名被工厂封装，客户端只依赖抽象接口
 *
 * 【缺点】
 * 1. 增加新产品时需要修改工厂接口，违反开闭原则（抽象工厂）
 * 2. 类的数量增加，系统复杂度上升
 * 3. 产品族扩展困难
 *
 * 【现代C++实现方式】
 * 1. 模板工厂 - 利用模板参数避免创建大量工厂类
 * 2. 反射式工厂 - 通过注册机制动态创建产品
 * 3. 智能指针管理产品生命周期
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <cassert>

// ============================================================
// 第一部分：工厂方法模式
// ============================================================

// 抽象产品
class IProduct {
public:
    virtual ~IProduct() = default;
    virtual std::string getName() const = 0;
    virtual void use() const = 0;
};

// 具体产品A
class ProductA : public IProduct {
public:
    ProductA() {
        std::cout << "  [ProductA] 创建" << std::endl;
    }
    ~ProductA() override {
        std::cout << "  [ProductA] 销毁" << std::endl;
    }
    std::string getName() const override { return "ProductA"; }
    void use() const override {
        std::cout << "  使用 ProductA 进行操作" << std::endl;
    }
};

// 具体产品B
class ProductB : public IProduct {
public:
    ProductB() {
        std::cout << "  [ProductB] 创建" << std::endl;
    }
    ~ProductB() override {
        std::cout << "  [ProductB] 销毁" << std::endl;
    }
    std::string getName() const override { return "ProductB"; }
    void use() const override {
        std::cout << "  使用 ProductB 进行操作" << std::endl;
    }
};

// 抽象工厂（工厂方法模式）
class IFactory {
public:
    virtual ~IFactory() = default;
    // 工厂方法：子类决定创建哪种产品
    virtual std::unique_ptr<IProduct> createProduct() = 0;
};

// 具体工厂A
class FactoryA : public IFactory {
public:
    std::unique_ptr<IProduct> createProduct() override {
        return std::make_unique<ProductA>();
    }
};

// 具体工厂B
class FactoryB : public IFactory {
public:
    std::unique_ptr<IProduct> createProduct() override {
        return std::make_unique<ProductB>();
    }
};

// ============================================================
// 模板工厂方法 - 避免为每个产品创建独立的工厂类
// ============================================================

/**
 * 利用C++模板，可以用一个模板工厂类代替所有具体工厂类。
 * 这是现代C++中常用的技巧，大大减少了类的数量。
 */
template <typename ProductT>
class TemplateFactory : public IFactory {
public:
    std::unique_ptr<IProduct> createProduct() override {
        return std::make_unique<ProductT>();
    }
};


// ============================================================
// 第二部分：抽象工厂模式
// ============================================================

// 抽象产品系列：按钮
class IButton {
public:
    virtual ~IButton() = default;
    virtual void render() const = 0;
    virtual std::string getStyle() const = 0;
};

// 抽象产品系列：文本框
class ITextBox {
public:
    virtual ~ITextBox() = default;
    virtual void render() const = 0;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getStyle() const = 0;
};

// 抽象产品系列：菜单
class IMenu {
public:
    virtual ~IMenu() = default;
    virtual void render() const = 0;
    virtual void addItem(const std::string& item) = 0;
    virtual std::string getStyle() const = 0;
};

// === 具体产品：Light主题 ===
class LightButton : public IButton {
public:
    void render() const override {
        std::cout << "  [LightButton] 渲染浅色按钮" << std::endl;
    }
    std::string getStyle() const override { return "Light"; }
};

class LightTextBox : public ITextBox {
public:
    void render() const override {
        std::cout << "  [LightTextBox] 渲染浅色文本框, 内容: " << text_ << std::endl;
    }
    void setText(const std::string& text) override { text_ = text; }
    std::string getStyle() const override { return "Light"; }
private:
    std::string text_;
};

class LightMenu : public IMenu {
public:
    void render() const override {
        std::cout << "  [LightMenu] 渲染浅色菜单, 菜单项数: " << items_.size() << std::endl;
    }
    void addItem(const std::string& item) override { items_.push_back(item); }
    std::string getStyle() const override { return "Light"; }
private:
    std::vector<std::string> items_;
};

// === 具体产品：Dark主题 ===
class DarkButton : public IButton {
public:
    void render() const override {
        std::cout << "  [DarkButton] 渲染深色按钮" << std::endl;
    }
    std::string getStyle() const override { return "Dark"; }
};

class DarkTextBox : public ITextBox {
public:
    void render() const override {
        std::cout << "  [DarkTextBox] 渲染深色文本框, 内容: " << text_ << std::endl;
    }
    void setText(const std::string& text) override { text_ = text; }
    std::string getStyle() const override { return "Dark"; }
private:
    std::string text_;
};

class DarkMenu : public IMenu {
public:
    void render() const override {
        std::cout << "  [DarkMenu] 渲染深色菜单, 菜单项数: " << items_.size() << std::endl;
    }
    void addItem(const std::string& item) override { items_.push_back(item); }
    std::string getStyle() const override { return "Dark"; }
private:
    std::vector<std::string> items_;
};

// 抽象工厂接口
class IGUIFactory {
public:
    virtual ~IGUIFactory() = default;
    virtual std::unique_ptr<IButton> createButton() = 0;
    virtual std::unique_ptr<ITextBox> createTextBox() = 0;
    virtual std::unique_ptr<IMenu> createMenu() = 0;
};

// 具体工厂：Light主题工厂
class LightThemeFactory : public IGUIFactory {
public:
    std::unique_ptr<IButton> createButton() override {
        return std::make_unique<LightButton>();
    }
    std::unique_ptr<ITextBox> createTextBox() override {
        return std::make_unique<LightTextBox>();
    }
    std::unique_ptr<IMenu> createMenu() override {
        return std::make_unique<LightMenu>();
    }
};

// 具体工厂：Dark主题工厂
class DarkThemeFactory : public IGUIFactory {
public:
    std::unique_ptr<IButton> createButton() override {
        return std::make_unique<DarkButton>();
    }
    std::unique_ptr<ITextBox> createTextBox() override {
        return std::make_unique<DarkTextBox>();
    }
    std::unique_ptr<IMenu> createMenu() override {
        return std::make_unique<DarkMenu>();
    }
};

// 客户端代码：只依赖抽象工厂和抽象产品
class Application {
public:
    explicit Application(std::unique_ptr<IGUIFactory> factory)
        : factory_(std::move(factory)) {
    }

    void buildUI() {
        button_ = factory_->createButton();
        textBox_ = factory_->createTextBox();
        menu_ = factory_->createMenu();

        textBox_->setText("欢迎使用应用");
        menu_->addItem("文件");
        menu_->addItem("编辑");
        menu_->addItem("帮助");
    }

    void render() const {
        std::cout << "=== 应用界面渲染 ===" << std::endl;
        button_->render();
        textBox_->render();
        menu_->render();
    }

    // 验证产品族一致性
    bool isThemeConsistent() const {
        return button_->getStyle() == textBox_->getStyle() &&
               textBox_->getStyle() == menu_->getStyle();
    }

private:
    std::unique_ptr<IGUIFactory> factory_;
    std::unique_ptr<IButton> button_;
    std::unique_ptr<ITextBox> textBox_;
    std::unique_ptr<IMenu> menu_;
};


// ============================================================
// 第三部分：反射式工厂（注册机制）
// ============================================================

/**
 * 反射式工厂：通过注册机制，可以在运行时动态创建产品。
 * 客户端通过字符串名称来创建对应的产品对象。
 *
 * 这是一种非常灵活的工厂实现，常用于插件系统、配置驱动的对象创建等场景。
 *
 * 核心思想：
 * 1. 用一个注册表（map）保存 "产品名 -> 创建函数" 的映射
 * 2. 每个具体产品在初始化时将自己注册到工厂
 * 3. 客户端通过名称查询并创建产品
 */
class ReflectiveFactory {
public:
    using CreatorFunc = std::function<std::unique_ptr<IProduct>()>;

    // 获取单例实例
    static ReflectiveFactory& instance() {
        static ReflectiveFactory factory;
        return factory;
    }

    // 注册产品
    bool registerProduct(const std::string& name, CreatorFunc creator) {
        auto result = registry_.emplace(name, std::move(creator));
        return result.second; // 返回是否注册成功
    }

    // 创建产品
    std::unique_ptr<IProduct> create(const std::string& name) {
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            return it->second();
        }
        return nullptr; // 未找到返回空
    }

    // 获取所有已注册的产品名
    std::vector<std::string> getRegisteredNames() const {
        std::vector<std::string> names;
        for (const auto& pair : registry_) {
            names.push_back(pair.first);
        }
        return names;
    }

private:
    ReflectiveFactory() = default;
    std::unordered_map<std::string, CreatorFunc> registry_;
};

// 辅助宏：用于自动注册产品
#define REGISTER_PRODUCT(ProductClass) \
    namespace { \
        bool _registered_##ProductClass = ReflectiveFactory::instance().registerProduct( \
            #ProductClass, \
            []() -> std::unique_ptr<IProduct> { \
                return std::make_unique<ProductClass>(); \
            } \
        ); \
    }

// 新的产品类型，自动注册
class ProductC : public IProduct {
public:
    ProductC() {
        std::cout << "  [ProductC] 创建" << std::endl;
    }
    std::string getName() const override { return "ProductC"; }
    void use() const override {
        std::cout << "  使用 ProductC 进行操作" << std::endl;
    }
};

REGISTER_PRODUCT(ProductC)

// ============================================================
// 测试代码
// ============================================================

void testFactoryMethod() {
    std::cout << "\n========== 工厂方法模式测试 ==========" << std::endl;

    // 使用具体工厂创建产品
    std::cout << "\n--- 使用 FactoryA ---" << std::endl;
    std::unique_ptr<IFactory> factoryA = std::make_unique<FactoryA>();
    auto productA = factoryA->createProduct();
    productA->use();

    std::cout << "\n--- 使用 FactoryB ---" << std::endl;
    std::unique_ptr<IFactory> factoryB = std::make_unique<FactoryB>();
    auto productB = factoryB->createProduct();
    productB->use();

    std::cout << "\n--- 使用模板工厂 ---" << std::endl;
    // 模板工厂：不需要为每个产品写独立的工厂类
    TemplateFactory<ProductA> templateFactoryA;
    auto product = templateFactoryA.createProduct();
    product->use();
}

void testAbstractFactory() {
    std::cout << "\n========== 抽象工厂模式测试 ==========" << std::endl;

    // Light主题应用
    std::cout << "\n--- Light 主题应用 ---" << std::endl;
    Application lightApp(std::make_unique<LightThemeFactory>());
    lightApp.buildUI();
    lightApp.render();
    std::cout << "主题一致性: " << (lightApp.isThemeConsistent() ? "是" : "否") << std::endl;

    // Dark主题应用
    std::cout << "\n--- Dark 主题应用 ---" << std::endl;
    Application darkApp(std::make_unique<DarkThemeFactory>());
    darkApp.buildUI();
    darkApp.render();
    std::cout << "主题一致性: " << (darkApp.isThemeConsistent() ? "是" : "否") << std::endl;
}

void testReflectiveFactory() {
    std::cout << "\n========== 反射式工厂测试 ==========" << std::endl;

    auto& factory = ReflectiveFactory::instance();

    // 手动注册ProductA和ProductB
    factory.registerProduct("ProductA", []() {
        return std::make_unique<ProductA>();
    });
    factory.registerProduct("ProductB", []() {
        return std::make_unique<ProductB>();
    });

    // 查看已注册的产品
    auto names = factory.getRegisteredNames();
    std::cout << "已注册的产品: ";
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    // 动态创建产品
    std::cout << "\n--- 动态创建 ProductA ---" << std::endl;
    auto p1 = factory.create("ProductA");
    if (p1) p1->use();

    std::cout << "\n--- 动态创建 ProductB ---" << std::endl;
    auto p2 = factory.create("ProductB");
    if (p2) p2->use();

    std::cout << "\n--- 动态创建 ProductC (通过宏自动注册) ---" << std::endl;
    auto p3 = factory.create("ProductC");
    if (p3) p3->use();

    std::cout << "\n--- 尝试创建不存在的产品 ---" << std::endl;
    auto p4 = factory.create("ProductX");
    std::cout << "创建结果: " << (p4 ? "成功" : "失败（产品不存在）") << std::endl;
}

int main() {
    std::cout << "========== 工厂模式学习 Demo ==========" << std::endl;

    testFactoryMethod();
    testAbstractFactory();
    testReflectiveFactory();

    std::cout << "\n========== 工厂模式总结 ==========" << std::endl;
    std::cout << "工厂方法：一个工厂创建一种产品，通过子类决定创建什么" << std::endl;
    std::cout << "抽象工厂：一个工厂创建一族相关产品，保证产品族一致性" << std::endl;
    std::cout << "模板工厂：利用模板减少工厂类数量，现代C++推荐" << std::endl;
    std::cout << "反射工厂：运行时动态注册和创建产品，灵活性最高" << std::endl;
    std::cout << "开闭原则：新增产品只需新增产品类，无需修改客户端代码" << std::endl;

    return 0;
}
