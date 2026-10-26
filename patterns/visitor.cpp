/**
 * 访问者模式 (Visitor Pattern)
 *
 * 【模式意图】
 * 表示一个作用于某对象结构中的各元素的操作。它使你可以在不改变各元素的类的前提下
 * 定义作用于这些元素的新操作。
 *
 * 【模式结构】
 * - Visitor: 抽象访问者，为每个ConcreteElement声明一个访问操作
 * - ConcreteVisitor: 具体访问者，实现每个访问操作
 * - Element: 抽象元素，定义一个accept操作，以一个访问者为参数
 * - ConcreteElement: 具体元素，实现accept操作
 * - ObjectStructure: 对象结构，能枚举它的元素
 *
 * 【双分派（Double Dispatch）】
 * 访问者模式的核心是双分派技术：
 * - 第一次分派：调用element.accept(visitor)，根据element的类型选择具体的accept方法
 * - 第二次分派：在accept中调用visitor.visit(*this)，根据visitor的类型选择具体的visit方法
 * 最终执行的操作取决于element和visitor两者的类型。
 *
 * 【适用场景】
 * 1. 一个对象结构包含很多类对象，它们有不同的接口，而你想对这些对象实施一些
 *    依赖于其具体类的操作
 * 2. 需要对一个对象结构中的对象进行很多不同的并且不相关的操作
 * 3. 当对象结构被很多不同的算法共享时
 * 4. 对象结构中的类很少改变，但经常需要在其上定义新的操作
 *
 * 【优点】
 * 1. 使得易于增加新的操作（只需添加新的访问者）
 * 2. 访问者将相关的行为集中到一个访问者对象中
 * 3. 访问者可以跨越类层次结构进行操作
 * 4. 可以累积状态
 *
 * 【缺点】
 * 1. 增加新的ConcreteElement很困难（需要修改所有Visitor）
 * 2. 可能破坏封装（访问者需要访问元素的内部状态）
 * 3. 双分派的概念较难理解
 *
 * 【现代C++实现方式】
 * 1. 经典访问者：虚函数双分派
 * 2. Acyclic Visitor：无循环依赖的访问者（使用多重继承）
 * 3. std::variant + std::visit：C++17的现代访问者模式
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <cmath>
#include <sstream>
#include <cassert>
#include <stdexcept>

// 圆周率：kPi 不是标准 C++ 宏（MSVC 需要 _USE_MATH_DEFINES），这里显式定义
constexpr double kPi = 3.14159265358979323846;

// ============================================================
// 经典访问者模式：表达式求值
// ============================================================

// 前向声明
class NumberExpr;
class AdditionExpr;
class SubtractionExpr;
class MultiplicationExpr;
class DivisionExpr;

// 抽象访问者
class IExpressionVisitor {
public:
    virtual ~IExpressionVisitor() = default;
    virtual void visit(NumberExpr& expr) = 0;
    virtual void visit(AdditionExpr& expr) = 0;
    virtual void visit(SubtractionExpr& expr) = 0;
    virtual void visit(MultiplicationExpr& expr) = 0;
    virtual void visit(DivisionExpr& expr) = 0;
};

// 抽象元素
class IExpression {
public:
    virtual ~IExpression() = default;
    virtual void accept(IExpressionVisitor& visitor) = 0;
};

// 具体元素：数字
class NumberExpr : public IExpression {
public:
    explicit NumberExpr(double value) : value_(value) {}

    void accept(IExpressionVisitor& visitor) override {
        visitor.visit(*this);  // 第二次分派
    }

    double getValue() const { return value_; }

private:
    double value_;
};

// 具体元素：加法
class AdditionExpr : public IExpression {
public:
    AdditionExpr(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    void accept(IExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }

    IExpression& getLeft() const { return *left_; }
    IExpression& getRight() const { return *right_; }

private:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
};

// 具体元素：减法
class SubtractionExpr : public IExpression {
public:
    SubtractionExpr(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    void accept(IExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }

    IExpression& getLeft() const { return *left_; }
    IExpression& getRight() const { return *right_; }

private:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
};

// 具体元素：乘法
class MultiplicationExpr : public IExpression {
public:
    MultiplicationExpr(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    void accept(IExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }

    IExpression& getLeft() const { return *left_; }
    IExpression& getRight() const { return *right_; }

private:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
};

// 具体元素：除法
class DivisionExpr : public IExpression {
public:
    DivisionExpr(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    void accept(IExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }

    IExpression& getLeft() const { return *left_; }
    IExpression& getRight() const { return *right_; }

private:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
};

// 具体访问者1：求值访问者
class EvalVisitor : public IExpressionVisitor {
public:
    void visit(NumberExpr& expr) override {
        result_ = expr.getValue();
    }

    void visit(AdditionExpr& expr) override {
        expr.getLeft().accept(*this);
        double leftVal = result_;
        expr.getRight().accept(*this);
        double rightVal = result_;
        result_ = leftVal + rightVal;
    }

    void visit(SubtractionExpr& expr) override {
        expr.getLeft().accept(*this);
        double leftVal = result_;
        expr.getRight().accept(*this);
        double rightVal = result_;
        result_ = leftVal - rightVal;
    }

    void visit(MultiplicationExpr& expr) override {
        expr.getLeft().accept(*this);
        double leftVal = result_;
        expr.getRight().accept(*this);
        double rightVal = result_;
        result_ = leftVal * rightVal;
    }

    void visit(DivisionExpr& expr) override {
        expr.getLeft().accept(*this);
        double leftVal = result_;
        expr.getRight().accept(*this);
        double rightVal = result_;
        if (rightVal == 0) {
            throw std::runtime_error("除数不能为零");
        }
        result_ = leftVal / rightVal;
    }

    double getResult() const { return result_; }

private:
    double result_ = 0;
};

// 具体访问者2：打印访问者
class PrintVisitor : public IExpressionVisitor {
public:
    void visit(NumberExpr& expr) override {
        output_ = std::to_string(expr.getValue());
        // 去除末尾多余的0
        output_.erase(output_.find_last_not_of('0') + 1, std::string::npos);
        if (output_.back() == '.') output_.pop_back();
    }

    void visit(AdditionExpr& expr) override {
        expr.getLeft().accept(*this);
        std::string leftStr = output_;
        expr.getRight().accept(*this);
        std::string rightStr = output_;
        output_ = "(" + leftStr + " + " + rightStr + ")";
    }

    void visit(SubtractionExpr& expr) override {
        expr.getLeft().accept(*this);
        std::string leftStr = output_;
        expr.getRight().accept(*this);
        std::string rightStr = output_;
        output_ = "(" + leftStr + " - " + rightStr + ")";
    }

    void visit(MultiplicationExpr& expr) override {
        expr.getLeft().accept(*this);
        std::string leftStr = output_;
        expr.getRight().accept(*this);
        std::string rightStr = output_;
        output_ = "(" + leftStr + " * " + rightStr + ")";
    }

    void visit(DivisionExpr& expr) override {
        expr.getLeft().accept(*this);
        std::string leftStr = output_;
        expr.getRight().accept(*this);
        std::string rightStr = output_;
        output_ = "(" + leftStr + " / " + rightStr + ")";
    }

    std::string getResult() const { return output_; }

private:
    std::string output_;
};

// ============================================================
// 2. Acyclic Visitor（无环访问者）
// ============================================================

/**
 * 经典访问者模式有一个问题：抽象访问者和具体元素之间形成了循环依赖。
 * Acyclic Visitor通过使用多重继承和dynamic_cast来打破这个循环。
 *
 * 优点：
 * - 增加新的元素类型不需要修改已有的访问者
 * - 没有循环依赖
 *
 * 缺点：
 * - 使用dynamic_cast有运行时开销
 * - 代码稍复杂
 */

// 抽象访问者基类（空接口）
class IAcyclicVisitor {
public:
    virtual ~IAcyclicVisitor() = default;
};

// 抽象元素
class IAcyclicElement {
public:
    virtual ~IAcyclicElement() = default;
    virtual void accept(IAcyclicVisitor& visitor) = 0;
    virtual std::string getType() const = 0;
};

// 前向声明具体元素，供访问者接口引用
class CircleElement;
class RectangleElement;
class TriangleElement;

// 各元素的访问者接口（独立接口，避免循环依赖）
// 必须在元素类之前定义，否则 dynamic_cast 的目标类型不完整
class ICircleVisitor {
public:
    virtual ~ICircleVisitor() = default;
    virtual void visitCircle(CircleElement& circle) = 0;
};

class IRectangleVisitor {
public:
    virtual ~IRectangleVisitor() = default;
    virtual void visitRectangle(RectangleElement& rect) = 0;
};

class ITriangleVisitor {
public:
    virtual ~ITriangleVisitor() = default;
    virtual void visitTriangle(TriangleElement& tri) = 0;
};

// 具体元素1：圆形
class CircleElement : public IAcyclicElement {
public:
    explicit CircleElement(double radius) : radius_(radius) {}

    void accept(IAcyclicVisitor& visitor) override {
        // 尝试将visitor转换为CircleVisitor
        if (auto* circleVisitor = dynamic_cast<ICircleVisitor*>(&visitor)) {
            circleVisitor->visitCircle(*this);
        }
        // 如果不支持，则不做任何事（静默跳过）
    }

    std::string getType() const override { return "圆形"; }
    double getRadius() const { return radius_; }

private:
    double radius_;
};

// 具体元素2：矩形
class RectangleElement : public IAcyclicElement {
public:
    RectangleElement(double width, double height)
        : width_(width), height_(height) {}

    void accept(IAcyclicVisitor& visitor) override {
        if (auto* rectVisitor = dynamic_cast<IRectangleVisitor*>(&visitor)) {
            rectVisitor->visitRectangle(*this);
        }
    }

    std::string getType() const override { return "矩形"; }
    double getWidth() const { return width_; }
    double getHeight() const { return height_; }

private:
    double width_;
    double height_;
};

// 具体元素3：三角形
class TriangleElement : public IAcyclicElement {
public:
    TriangleElement(double base, double height)
        : base_(base), height_(height) {}

    void accept(IAcyclicVisitor& visitor) override {
        if (auto* triVisitor = dynamic_cast<ITriangleVisitor*>(&visitor)) {
            triVisitor->visitTriangle(*this);
        }
    }

    std::string getType() const override { return "三角形"; }
    double getBase() const { return base_; }
    double getHeight() const { return height_; }

private:
    double base_;
    double height_;
};

// 具体访问者：面积计算（只关心圆形和矩形，不关心三角形）
class AreaVisitor : public IAcyclicVisitor,
                   public ICircleVisitor,
                   public IRectangleVisitor {
public:
    void visitCircle(CircleElement& circle) override {
        double area = kPi * circle.getRadius() * circle.getRadius();
        totalArea_ += area;
        std::cout << "  [AreaVisitor] 圆形面积: " << area << std::endl;
    }

    void visitRectangle(RectangleElement& rect) override {
        double area = rect.getWidth() * rect.getHeight();
        totalArea_ += area;
        std::cout << "  [AreaVisitor] 矩形面积: " << area << std::endl;
    }

    // 注意：没有实现visitTriangle，说明不处理三角形

    double getTotalArea() const { return totalArea_; }

private:
    double totalArea_ = 0;
};

// ============================================================
// 3. std::variant + std::visit（C++17现代访问者）
// ============================================================

/**
 * C++17引入了std::variant和std::visit，提供了一种内置的访问者模式实现。
 * 这是一种更现代、更类型安全的方式，不需要虚函数和继承。
 *
 * 核心思想：
 * - std::variant<A, B, C> 是一个可以存放A、B、C中任意一种类型的联合体
 * - std::visit(visitor, variant) 根据variant中实际存放的类型调用对应的visitor
 *
 * 优点：
 * - 不需要继承体系，类型可以是任意的（甚至是基本类型）
 * - 编译期检查所有类型是否都被处理
 * - 性能更好（没有虚函数调用开销）
 * - 符合"闭类型"的概念，类型集合在编译期确定
 */

// 不同的形状类型（不需要继承自共同基类）
struct CircleV {
    double radius;
};

struct RectangleV {
    double width;
    double height;
};

struct TriangleV {
    double base;
    double height;
};

// 使用variant定义形状类型
using ShapeV = std::variant<CircleV, RectangleV, TriangleV>;

// 访问者1：面积计算（使用函数对象的重载）
struct AreaVisitorV {
    double operator()(const CircleV& c) const {
        return kPi * c.radius * c.radius;
    }

    double operator()(const RectangleV& r) const {
        return r.width * r.height;
    }

    double operator()(const TriangleV& t) const {
        return 0.5 * t.base * t.height;
    }
};

// 访问者2：周长计算
struct PerimeterVisitorV {
    double operator()(const CircleV& c) const {
        return 2 * kPi * c.radius;
    }

    double operator()(const RectangleV& r) const {
        return 2 * (r.width + r.height);
    }

    double operator()(const TriangleV& t) const {
        // 简化：假设是等腰三角形
        double side = std::sqrt(t.base * t.base / 4 + t.height * t.height);
        return t.base + 2 * side;
    }
};

// 访问者3：打印形状信息
struct PrintVisitorV {
    std::string operator()(const CircleV& c) const {
        return "圆形(半径=" + std::to_string(c.radius) + ")";
    }

    std::string operator()(const RectangleV& r) const {
        return "矩形(宽=" + std::to_string(r.width) +
               ", 高=" + std::to_string(r.height) + ")";
    }

    std::string operator()(const TriangleV& t) const {
        return "三角形(底=" + std::to_string(t.base) +
               ", 高=" + std::to_string(t.height) + ")";
    }
};

// 模板技巧：用于创建重载的lambda访问者
template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;  // 推导指引

// ============================================================
// 辅助函数：构建表达式
// ============================================================

std::unique_ptr<IExpression> makeNumber(double v) {
    return std::make_unique<NumberExpr>(v);
}

std::unique_ptr<IExpression> makeAdd(std::unique_ptr<IExpression> l,
                                     std::unique_ptr<IExpression> r) {
    return std::make_unique<AdditionExpr>(std::move(l), std::move(r));
}

std::unique_ptr<IExpression> makeSub(std::unique_ptr<IExpression> l,
                                     std::unique_ptr<IExpression> r) {
    return std::make_unique<SubtractionExpr>(std::move(l), std::move(r));
}

std::unique_ptr<IExpression> makeMul(std::unique_ptr<IExpression> l,
                                     std::unique_ptr<IExpression> r) {
    return std::make_unique<MultiplicationExpr>(std::move(l), std::move(r));
}

std::unique_ptr<IExpression> makeDiv(std::unique_ptr<IExpression> l,
                                     std::unique_ptr<IExpression> r) {
    return std::make_unique<DivisionExpr>(std::move(l), std::move(r));
}

// ============================================================
// 测试代码
// ============================================================

void testClassicVisitor() {
    std::cout << "\n========== 经典访问者模式测试 ==========" << std::endl;

    // 构建表达式: (3 + 4) * (5 - 2) = 7 * 3 = 21
    auto expr = makeMul(
        makeAdd(makeNumber(3), makeNumber(4)),
        makeSub(makeNumber(5), makeNumber(2))
    );

    // 打印表达式
    PrintVisitor printer;
    expr->accept(printer);
    std::cout << "  表达式: " << printer.getResult() << std::endl;

    // 求值
    EvalVisitor eval;
    expr->accept(eval);
    std::cout << "  结果: " << eval.getResult() << std::endl;

    // 另一个表达式: 10 / (1 + 1) = 5
    std::cout << "\n--- 另一个表达式 ---" << std::endl;
    auto expr2 = makeDiv(
        makeNumber(10),
        makeAdd(makeNumber(1), makeNumber(1))
    );
    expr2->accept(printer);
    std::cout << "  表达式: " << printer.getResult() << std::endl;
    expr2->accept(eval);
    std::cout << "  结果: " << eval.getResult() << std::endl;

    std::cout << "\n  双分派：操作取决于元素类型和访问者类型两者" << std::endl;
}

void testAcyclicVisitor() {
    std::cout << "\n========== Acyclic Visitor测试 ==========" << std::endl;

    std::vector<std::unique_ptr<IAcyclicElement>> shapes;
    shapes.push_back(std::make_unique<CircleElement>(5.0));
    shapes.push_back(std::make_unique<RectangleElement>(4.0, 6.0));
    shapes.push_back(std::make_unique<TriangleElement>(3.0, 4.0));
    shapes.push_back(std::make_unique<CircleElement>(2.0));

    AreaVisitor areaVisitor;
    std::cout << "\n  计算面积（AreaVisitor不处理三角形）:" << std::endl;
    for (auto& shape : shapes) {
        std::cout << "  处理 " << shape->getType() << ":" << std::endl;
        shape->accept(areaVisitor);
    }
    std::cout << "  总面积（仅圆形+矩形）: " << areaVisitor.getTotalArea() << std::endl;

    std::cout << "\n  Acyclic Visitor的优势：" << std::endl;
    std::cout << "  - 新增元素类型不需要修改已有访问者" << std::endl;
    std::cout << "  - 访问者可以选择只处理感兴趣的类型" << std::endl;
    std::cout << "  - 没有循环依赖" << std::endl;
}

void testVariantVisit() {
    std::cout << "\n========== std::variant + std::visit测试 ==========" << std::endl;

    std::vector<ShapeV> shapes = {
        CircleV{5.0},
        RectangleV{4.0, 6.0},
        TriangleV{3.0, 4.0},
        CircleV{2.5}
    };

    // 打印所有形状
    std::cout << "\n--- 形状列表 ---" << std::endl;
    for (const auto& shape : shapes) {
        std::cout << "  " << std::visit(PrintVisitorV{}, shape) << std::endl;
    }

    // 计算总面积
    std::cout << "\n--- 面积计算 ---" << std::endl;
    double totalArea = 0;
    for (const auto& shape : shapes) {
        double area = std::visit(AreaVisitorV{}, shape);
        std::cout << "  面积: " << area << std::endl;
        totalArea += area;
    }
    std::cout << "  总面积: " << totalArea << std::endl;

    // 使用lambda组合访问者（Overloaded技巧）
    std::cout << "\n--- 使用lambda组合访问者 ---" << std::endl;
    auto lambdaVisitor = Overloaded{
        [](const CircleV& c) {
            return "Lambda访问: 圆形，面积=" + std::to_string(kPi * c.radius * c.radius);
        },
        [](const RectangleV& r) {
            return "Lambda访问: 矩形，面积=" + std::to_string(r.width * r.height);
        },
        [](const TriangleV& t) {
            return "Lambda访问: 三角形，面积=" + std::to_string(0.5 * t.base * t.height);
        }
    };

    for (const auto& shape : shapes) {
        std::cout << "  " << std::visit(lambdaVisitor, shape) << std::endl;
    }

    std::cout << "\n  std::variant的优势：" << std::endl;
    std::cout << "  - 不需要继承体系" << std::endl;
    std::cout << "  - 编译期检查所有类型是否都被处理" << std::endl;
    std::cout << "  - 性能更好（无虚函数调用）" << std::endl;
    std::cout << "  - 类型集合在编译期确定（闭类型）" << std::endl;
}

int main() {
    std::cout << "========== 访问者模式学习 Demo ==========" << std::endl;

    testClassicVisitor();
    testAcyclicVisitor();
    testVariantVisit();

    std::cout << "\n========== 访问者模式总结 ==========" << std::endl;
    std::cout << "核心思想：将数据结构与操作分离，在不修改类的前提下添加新操作" << std::endl;
    std::cout << "双分派：操作取决于元素和访问者两者的类型" << std::endl;
    std::cout << "经典访问者：虚函数实现，元素类稳定、操作多变时使用" << std::endl;
    std::cout << "Acyclic Visitor：打破循环依赖，使用dynamic_cast" << std::endl;
    std::cout << "std::variant：C++17现代方式，编译期检查，性能最优" << std::endl;
    std::cout << "适用场景：类结构稳定但操作经常变化" << std::endl;
    std::cout << "设计原则：开闭原则（对操作开放，对类关闭）" << std::endl;

    return 0;
}
