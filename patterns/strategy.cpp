/**
 * 策略模式 (Strategy Pattern)
 *
 * 【模式意图】
 * 定义一系列的算法，把它们一个个封装起来，并且使它们可相互替换。本模式使得算法
 * 可独立于使用它的客户而变化。
 *
 * 【模式结构】
 * - Strategy: 抽象策略，定义所有支持的算法的公共接口
 * - ConcreteStrategy: 具体策略，实现具体的算法
 * - Context: 上下文，用一个ConcreteStrategy来配置，维护一个对Strategy对象的引用
 *
 * 【适用场景】
 * 1. 许多相关的类仅仅是行为有异，需要用多种行为中的一个来配置一个类
 * 2. 需要使用一个算法的不同变体
 * 3. 算法使用客户不应该知道的数据
 * 4. 一个类定义了多种行为，并且这些行为在这个类的操作中以多个条件语句的形式出现
 *
 * 【优点】
 * 1. 相关算法系列被封装在类层次结构中，易于管理和扩展
 * 2. 提供了一种可以替代继承的方法
 * 3. 消除了一些条件语句
 * 4. 可以在运行时切换策略
 * 5. 符合开闭原则
 *
 * 【缺点】
 * 1. 客户端必须知道所有的策略类，并自行决定使用哪一个
 * 2. 策略模式将造成产生很多策略类
 *
 * 【与模板方法的区别】
 * - 策略模式：使用组合，算法在策略对象中，运行时可切换
 * - 模板方法：使用继承，算法在父类中，子类实现具体步骤
 *
 * 【现代C++实现方式】
 * 1. 经典策略：虚函数接口
 * 2. std::function策略：函数式风格，轻量级
 * 3. 模板策略：编译期策略选择，零开销
 * 4. 策略组合：多个策略组合使用
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>
#include <chrono>
#include <cassert>
#include <sstream>

// ============================================================
// 经典策略模式：排序策略
// ============================================================

// 抽象策略：排序算法
class ISortStrategy {
public:
    virtual ~ISortStrategy() = default;
    virtual void sort(std::vector<int>& data) const = 0;
    virtual std::string getName() const = 0;
};

// 具体策略：冒泡排序
class BubbleSort : public ISortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        int n = static_cast<int>(data.size());
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n - i - 1; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
    std::string getName() const override { return "冒泡排序"; }
};

// 具体策略：快速排序
class QuickSort : public ISortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        quickSort(data, 0, static_cast<int>(data.size()) - 1);
    }
    std::string getName() const override { return "快速排序"; }

private:
    void quickSort(std::vector<int>& data, int low, int high) const {
        if (low < high) {
            int pi = partition(data, low, high);
            quickSort(data, low, pi - 1);
            quickSort(data, pi + 1, high);
        }
    }
    int partition(std::vector<int>& data, int low, int high) const {
        int pivot = data[high];
        int i = low - 1;
        for (int j = low; j < high; ++j) {
            if (data[j] <= pivot) {
                i++;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[high]);
        return i + 1;
    }
};

// 具体策略：归并排序
class MergeSort : public ISortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        mergeSort(data, 0, static_cast<int>(data.size()) - 1);
    }
    std::string getName() const override { return "归并排序"; }

private:
    void mergeSort(std::vector<int>& data, int left, int right) const {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(data, left, mid);
            mergeSort(data, mid + 1, right);
            merge(data, left, mid, right);
        }
    }
    void merge(std::vector<int>& data, int left, int mid, int right) const {
        std::vector<int> temp(right - left + 1);
        int i = left, j = mid + 1, k = 0;
        while (i <= mid && j <= right) {
            if (data[i] <= data[j]) temp[k++] = data[i++];
            else temp[k++] = data[j++];
        }
        while (i <= mid) temp[k++] = data[i++];
        while (j <= right) temp[k++] = data[j++];
        for (int p = 0; p < k; ++p) data[left + p] = temp[p];
    }
};

// 上下文：排序器
class Sorter {
public:
    explicit Sorter(std::unique_ptr<ISortStrategy> strategy)
        : strategy_(std::move(strategy)) {
    }

    // 运行时切换策略
    void setStrategy(std::unique_ptr<ISortStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    void sortData(std::vector<int>& data) {
        std::cout << "  使用 " << strategy_->getName() << " 对 "
                  << data.size() << " 个元素排序" << std::endl;
        strategy_->sort(data);
    }

private:
    std::unique_ptr<ISortStrategy> strategy_;
};

// ============================================================
// 2. std::function 策略（函数式风格）
// ============================================================

/**
 * 使用std::function实现策略模式，不需要定义抽象基类和具体策略类。
 * 任何可调用对象（函数、lambda、函数对象）都可以作为策略。
 *
 * 优点：
 * - 代码更简洁，不需要类层次结构
 * - 可以直接使用lambda定义策略
 * - 可以捕获外部变量
 */

// 支付策略类型
using PaymentStrategy = std::function<double(double amount)>;

// 支付处理器（上下文）
class PaymentProcessor {
public:
    void setPaymentMethod(PaymentStrategy strategy) {
        strategy_ = std::move(strategy);
    }

    double processPayment(double amount) {
        if (!strategy_) {
            throw std::runtime_error("未设置支付方式");
        }
        double finalAmount = strategy_(amount);
        std::cout << "  支付处理完成，实际收取: ¥" << finalAmount << std::endl;
        return finalAmount;
    }

private:
    PaymentStrategy strategy_;
};

// 各种支付策略（函数或lambda）
double creditCardPayment(double amount) {
    double fee = amount * 0.02; // 2%手续费
    std::cout << "  [信用卡支付] 金额: ¥" << amount
              << ", 手续费: ¥" << fee << std::endl;
    return amount + fee;
}

double alipayPayment(double amount) {
    double discount = amount > 100 ? 5 : 0; // 满100减5
    std::cout << "  [支付宝支付] 金额: ¥" << amount
              << ", 优惠: ¥" << discount << std::endl;
    return amount - discount;
}

// 策略工厂
PaymentStrategy createWechatPayStrategy() {
    return [](double amount) {
        double fee = 0.1; // 固定手续费
        std::cout << "  [微信支付] 金额: ¥" << amount
                  << ", 手续费: ¥" << fee << std::endl;
        return amount + fee;
    };
}

// ============================================================
// 3. 模板策略（编译期策略选择）
// ============================================================

/**
 * 使用模板参数选择策略，在编译期确定，零运行时开销。
 * 这是C++中非常常用的技术，STL中大量使用（如allocator、traits等）。
 *
 * 优点：
 * - 零运行时开销，无虚函数调用
 * - 编译器可以进行更多优化（内联等）
 * - 类型安全
 */

// 日志策略
struct ConsoleLogPolicy {
    void log(const std::string& message) const {
        std::cout << "  [ConsoleLog] " << message << std::endl;
    }
};

struct FileLogPolicy {
    void log(const std::string& message) const {
        // 实际会写入文件，这里模拟
        std::cout << "  [FileLog] 写入文件: " << message << std::endl;
    }
};

struct NullLogPolicy {
    void log(const std::string&) const {
        // 空实现，用于发布版本禁用日志
    }
};

// 模板类：使用日志策略
template <typename LogPolicy>
class UserService_Template {
public:
    void createUser(const std::string& username) {
        logPolicy_.log("创建用户: " + username);
        // ... 实际创建逻辑
    }

    void deleteUser(int userId) {
        logPolicy_.log("删除用户: ID=" + std::to_string(userId));
        // ... 实际删除逻辑
    }

private:
    LogPolicy logPolicy_;
};

// ============================================================
// 4. 策略组合
// ============================================================

/**
 * 有时候一个问题需要多个策略协同工作。
 * 比如文本格式化可能需要：缩进策略 + 换行策略 + 编码策略
 */

// 缩进策略
class IIndentStrategy {
public:
    virtual ~IIndentStrategy() = default;
    virtual std::string getIndent(int level) const = 0;
};

class SpaceIndent : public IIndentStrategy {
public:
    explicit SpaceIndent(int spaces = 4) : spaces_(spaces) {}
    std::string getIndent(int level) const override {
        return std::string(level * spaces_, ' ');
    }
private:
    int spaces_;
};

class TabIndent : public IIndentStrategy {
public:
    std::string getIndent(int level) const override {
        return std::string(level, '\t');
    }
};

// 换行策略
class INewlineStrategy {
public:
    virtual ~INewlineStrategy() = default;
    virtual std::string getNewline() const = 0;
};

class UnixNewline : public INewlineStrategy {
public:
    std::string getNewline() const override { return "\n"; }
};

class WindowsNewline : public INewlineStrategy {
public:
    std::string getNewline() const override { return "\r\n"; }
};

// 上下文：代码格式化器，组合多个策略
class CodeFormatter {
public:
    CodeFormatter(std::unique_ptr<IIndentStrategy> indent,
                  std::unique_ptr<INewlineStrategy> newline)
        : indentStrategy_(std::move(indent))
        , newlineStrategy_(std::move(newline)) {
    }

    void setIndentStrategy(std::unique_ptr<IIndentStrategy> strategy) {
        indentStrategy_ = std::move(strategy);
    }

    void setNewlineStrategy(std::unique_ptr<INewlineStrategy> strategy) {
        newlineStrategy_ = std::move(strategy);
    }

    std::string formatCode(const std::vector<std::string>& lines) {
        std::ostringstream oss;
        int indentLevel = 0;
        for (const auto& line : lines) {
            if (line == "}") indentLevel = std::max(0, indentLevel - 1);
            oss << indentStrategy_->getIndent(indentLevel)
                << line
                << newlineStrategy_->getNewline();
            if (line == "{") indentLevel++;
        }
        return oss.str();
    }

private:
    std::unique_ptr<IIndentStrategy> indentStrategy_;
    std::unique_ptr<INewlineStrategy> newlineStrategy_;
};

// ============================================================
// 5. 策略模式 vs 模板方法模式
// ============================================================

/**
 * 策略模式和模板方法模式都用于封装算法变体，但实现方式不同：
 *
 * 策略模式（组合）：
 * - 通过组合不同的策略对象来改变行为
 * - 运行时可以切换策略
 * - 策略之间完全独立
 * - 符合"组合优于继承"
 *
 * 模板方法（继承）：
 * - 通过继承改变算法的某些步骤
 * - 编译时确定，不能运行时切换
 * - 子类可以访问父类的 protected 成员
 * - 代码复用性更高（公共部分在父类）
 */

// 模板方法示例
class DataProcessor {
public:
    void process(const std::string& input) {
        std::string data = preprocess(input);    // 步骤1（可能被重写）
        data = transform(data);                  // 步骤2（必须重写）
        postprocess(data);                        // 步骤3（可能被重写）
    }

    virtual ~DataProcessor() = default;

protected:
    // 钩子方法（Hook）：有默认实现，子类可选择性重写
    virtual std::string preprocess(const std::string& input) {
        return input; // 默认不做预处理
    }

    // 抽象方法：必须由子类实现
    virtual std::string transform(const std::string& data) = 0;

    // 钩子方法
    virtual void postprocess(const std::string& data) {
        std::cout << "  [模板方法] 处理结果: " << data << std::endl;
    }
};

// 具体子类
class UpperCaseProcessor : public DataProcessor {
protected:
    std::string transform(const std::string& data) override {
        std::string result = data;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
};

class TrimProcessor : public DataProcessor {
protected:
    std::string preprocess(const std::string& input) override {
        // 重写preprocess钩子
        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : input.substr(start, end - start + 1);
    }

    std::string transform(const std::string& data) override {
        return "[" + data + "]";
    }
};

// ============================================================
// 测试代码
// ============================================================

void testClassicStrategy() {
    std::cout << "\n========== 经典策略模式测试 ==========" << std::endl;

    Sorter sorter(std::make_unique<BubbleSort>());

    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90, 45, 33, 77};
    auto dataCopy = data;

    // 冒泡排序
    std::cout << "\n--- 冒泡排序 ---" << std::endl;
    sorter.sortData(dataCopy);

    // 切换为快速排序
    std::cout << "\n--- 切换为快速排序 ---" << std::endl;
    dataCopy = data;
    sorter.setStrategy(std::make_unique<QuickSort>());
    sorter.sortData(dataCopy);

    // 切换为归并排序
    std::cout << "\n--- 切换为归并排序 ---" << std::endl;
    dataCopy = data;
    sorter.setStrategy(std::make_unique<MergeSort>());
    sorter.sortData(dataCopy);
}

void testFunctionStrategy() {
    std::cout << "\n========== std::function策略测试 ==========" << std::endl;

    PaymentProcessor processor;

    // 信用卡支付
    std::cout << "\n--- 信用卡支付 ---" << std::endl;
    processor.setPaymentMethod(creditCardPayment);
    processor.processPayment(200.0);

    // 支付宝支付
    std::cout << "\n--- 支付宝支付 ---" << std::endl;
    processor.setPaymentMethod(alipayPayment);
    processor.processPayment(150.0);

    // 微信支付（工厂创建）
    std::cout << "\n--- 微信支付 ---" << std::endl;
    processor.setPaymentMethod(createWechatPayStrategy());
    processor.processPayment(50.0);

    // 直接使用lambda定义策略
    std::cout << "\n--- 优惠券策略（lambda） ---" << std::endl;
    double couponValue = 20.0;
    processor.setPaymentMethod([couponValue](double amount) {
        double finalAmount = std::max(0.0, amount - couponValue);
        std::cout << "  [优惠券支付] 金额: ¥" << amount
                  << ", 优惠券抵扣: ¥" << couponValue << std::endl;
        return finalAmount;
    });
    processor.processPayment(100.0);
}

void testTemplateStrategy() {
    std::cout << "\n========== 模板策略测试（编译期选择） ==========" << std::endl;

    // 控制台日志版本
    std::cout << "\n--- 控制台日志版本 ---" << std::endl;
    UserService_Template<ConsoleLogPolicy> consoleService;
    consoleService.createUser("Alice");
    consoleService.deleteUser(123);

    // 文件日志版本
    std::cout << "\n--- 文件日志版本 ---" << std::endl;
    UserService_Template<FileLogPolicy> fileService;
    fileService.createUser("Bob");

    // 空日志版本（发布版本）
    std::cout << "\n--- 空日志版本（无输出） ---" << std::endl;
    UserService_Template<NullLogPolicy> nullService;
    nullService.createUser("Charlie");
    std::cout << "  (空日志策略：无日志输出，零开销)" << std::endl;

    std::cout << "\n  模板策略在编译期确定，零运行时虚函数开销" << std::endl;
}

void testStrategyComposition() {
    std::cout << "\n========== 策略组合测试 ==========" << std::endl;

    std::vector<std::string> code = {
        "class Test {",
        "void foo() {",
        "int x = 1;",
        "}",
        "}"
    };

    // 4空格 + Unix换行
    std::cout << "\n--- 4空格缩进 + Unix换行 ---" << std::endl;
    CodeFormatter formatter1(
        std::make_unique<SpaceIndent>(4),
        std::make_unique<UnixNewline>()
    );
    std::string result1 = formatter1.formatCode(code);
    std::cout << result1;

    // Tab + Windows换行
    std::cout << "\n--- Tab缩进 + Windows换行 ---" << std::endl;
    CodeFormatter formatter2(
        std::make_unique<TabIndent>(),
        std::make_unique<WindowsNewline>()
    );
    std::string result2 = formatter2.formatCode(code);
    // 显示\r\n的可见形式
    std::string visible;
    for (char c : result2) {
        if (c == '\r') visible += "\\r";
        else if (c == '\n') visible += "\\n\n";
        else if (c == '\t') visible += "→";
        else visible += c;
    }
    std::cout << visible;

    // 运行时切换策略
    std::cout << "\n--- 运行时切换为2空格缩进 ---" << std::endl;
    formatter1.setIndentStrategy(std::make_unique<SpaceIndent>(2));
    std::cout << formatter1.formatCode(code);
}

void testTemplateMethod() {
    std::cout << "\n========== 模板方法模式对比 ==========" << std::endl;

    std::cout << "\n--- UpperCaseProcessor ---" << std::endl;
    UpperCaseProcessor upperProc;
    upperProc.process("hello world");

    std::cout << "\n--- TrimProcessor ---" << std::endl;
    TrimProcessor trimProc;
    trimProc.process("   hello   ");

    std::cout << "\n  策略模式 vs 模板方法:" << std::endl;
    std::cout << "  策略模式: 组合, 运行时切换, 更灵活" << std::endl;
    std::cout << "  模板方法: 继承, 编译时确定, 代码复用性高" << std::endl;
}

int main() {
    std::cout << "========== 策略模式学习 Demo ==========" << std::endl;

    testClassicStrategy();
    testFunctionStrategy();
    testTemplateStrategy();
    testStrategyComposition();
    testTemplateMethod();

    std::cout << "\n========== 策略模式总结 ==========" << std::endl;
    std::cout << "核心思想：封装算法族，使它们可互相替换" << std::endl;
    std::cout << "经典策略：虚函数接口，运行时切换" << std::endl;
    std::cout << "std::function策略：函数式风格，轻量灵活" << std::endl;
    std::cout << "模板策略：编译期选择，零运行时开销" << std::endl;
    std::cout << "策略组合：多个策略协同工作" << std::endl;
    std::cout << "vs模板方法：策略用组合，模板方法用继承" << std::endl;
    std::cout << "设计原则：开闭原则、组合优于继承" << std::endl;

    return 0;
}
