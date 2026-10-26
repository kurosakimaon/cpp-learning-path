/**
 * 状态模式 (State Pattern)
 *
 * 【模式意图】
 * 允许一个对象在其内部状态改变时改变它的行为，对象看起来似乎修改了它的类。
 *
 * 【模式结构】
 * - Context: 环境类，定义客户感兴趣的接口，维护一个ConcreteState子类的实例
 * - State: 抽象状态，定义一个接口以封装与Context的一个特定状态相关的行为
 * - ConcreteState: 具体状态，每一个子类实现一个与Context的一个状态相关的行为
 *
 * 【适用场景】
 * 1. 一个对象的行为取决于它的状态，并且它必须在运行时刻根据状态改变它的行为
 * 2. 一个操作中含有庞大的多分支的条件语句，且这些分支依赖于该对象的状态
 * 3. 对象有大量的状态转换逻辑散落在各处
 *
 * 【优点】
 * 1. 将与特定状态相关的行为局部化，使得状态转换显式化
 * 2. 消除了庞大的条件分支语句
 * 3. 状态对象可以被共享（享元）
 * 4. 状态相关行为局部化，新增状态类本身不需要改动已有状态类
 *
 * 【缺点】
 * 1. 增加了状态类的数量，系统类和对象的个数都会增加
 * 2. 对开闭原则的支持有限：新增状态往往需要修改其他状态类中的转换逻辑
 * 3. 状态模式的结构与实现都较为复杂
 *
 * 【与策略模式的区别】
 * - 状态模式：状态之间有转换关系，状态知道自己要转换到哪个状态
 * - 策略模式：各个策略是平等独立的，没有转换关系，由客户端切换
 * - 状态模式：解决的是对象内部状态变化导致行为变化的问题
 * - 策略模式：解决的是算法替换的问题
 *
 * 【现代C++实现方式】
 * 1. 经典状态模式：状态类封装行为和转换
 * 2. 状态转换表：表格驱动的状态机
 * 3. 枚举+函数表：轻量级状态机
 * 4. 游戏AI状态机：实际应用示例
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cassert>
#include <sstream>

// ============================================================
// 经典状态模式：订单状态机
// ============================================================

// 前向声明
class Order;
enum class OrderEvent;

// 抽象状态
class IOrderState {
public:
    virtual ~IOrderState() = default;
    virtual std::string getName() const = 0;
    virtual void pay(Order& order) = 0;       // 支付
    virtual void cancel(Order& order) = 0;    // 取消
    virtual void ship(Order& order) = 0;      // 发货
    virtual void deliver(Order& order) = 0;   // 签收
    virtual void refund(Order& order) = 0;    // 退款
};

// 状态工厂：定义在具体状态类之后，供状态转换使用
std::shared_ptr<IOrderState> paidState();
std::shared_ptr<IOrderState> cancelledState();
std::shared_ptr<IOrderState> shippedState();
std::shared_ptr<IOrderState> deliveredState();
std::shared_ptr<IOrderState> refundingState();
std::shared_ptr<IOrderState> refundedState();

// 订单上下文
class Order {
public:
    Order() : state_(nullptr) {}

    void setState(std::shared_ptr<IOrderState> state) {
        if (!state) {
            std::cout << "  [订单状态] 错误：状态不能为空，忽略本次设置" << std::endl;
            return;
        }
        if (state_) {
            std::cout << "  [订单状态] " << state_->getName() << " -> "
                      << state->getName() << std::endl;
        } else {
            std::cout << "  [订单状态] 初始状态: " << state->getName() << std::endl;
        }
        state_ = std::move(state);
    }

    std::string getStateName() const {
        return state_ ? state_->getName() : "无状态";
    }

    // 操作委托给状态对象。未设置状态时直接报错，
    // 而不是解引用空指针；状态对象内部也可能调用 setState。
    bool hasState() const { return static_cast<bool>(state_); }
    void pay() { if (requireState("支付")) state_->pay(*this); }
    void cancel() { if (requireState("取消")) state_->cancel(*this); }
    void ship() { if (requireState("发货")) state_->ship(*this); }
    void deliver() { if (requireState("签收")) state_->deliver(*this); }
    void refund() { if (requireState("退款")) state_->refund(*this); }

private:
    bool requireState(const char* action) const {
        if (!state_) {
            std::cout << "  [操作] 错误：订单尚未设置状态，无法" << action << std::endl;
            return false;
        }
        return true;
    }

    std::shared_ptr<IOrderState> state_;
};

// 具体状态：待支付
class PendingPaymentState : public IOrderState,
                            public std::enable_shared_from_this<PendingPaymentState> {
public:
    std::string getName() const override { return "待支付"; }

    void pay(Order& order) override {
        std::cout << "  [操作] 支付成功" << std::endl;
        order.setState(paidState());
    }

    void cancel(Order& order) override {
        std::cout << "  [操作] 取消订单" << std::endl;
        order.setState(cancelledState());
    }

    void ship(Order&) override {
        std::cout << "  [操作] 错误：未支付不能发货" << std::endl;
    }

    void deliver(Order&) override {
        std::cout << "  [操作] 错误：未支付不能签收" << std::endl;
    }

    void refund(Order&) override {
        std::cout << "  [操作] 错误：未支付不能退款" << std::endl;
    }

    // 单例（享元模式：状态对象可以被共享）
    static std::shared_ptr<PendingPaymentState> instance() {
        static auto inst = std::shared_ptr<PendingPaymentState>(new PendingPaymentState());
        return inst;
    }

private:
    PendingPaymentState() = default;
};

// 具体状态：已支付
class PaidState : public IOrderState,
                  public std::enable_shared_from_this<PaidState> {
public:
    std::string getName() const override { return "已支付"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：已支付，不能重复支付" << std::endl;
    }

    void cancel(Order& order) override {
        std::cout << "  [操作] 取消订单，发起退款" << std::endl;
        order.setState(refundingState());
    }

    void ship(Order& order) override {
        std::cout << "  [操作] 发货" << std::endl;
        order.setState(shippedState());
    }

    void deliver(Order&) override {
        std::cout << "  [操作] 错误：未发货不能签收" << std::endl;
    }

    void refund(Order& order) override {
        std::cout << "  [操作] 申请退款" << std::endl;
        order.setState(refundingState());
    }

    static std::shared_ptr<PaidState> instance() {
        static auto inst = std::shared_ptr<PaidState>(new PaidState());
        return inst;
    }

private:
    PaidState() = default;
};

// 具体状态：已发货
class ShippedState : public IOrderState,
                     public std::enable_shared_from_this<ShippedState> {
public:
    std::string getName() const override { return "已发货"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：已支付" << std::endl;
    }

    void cancel(Order&) override {
        std::cout << "  [操作] 错误：已发货，不能取消，请申请退款" << std::endl;
    }

    void ship(Order&) override {
        std::cout << "  [操作] 错误：已发货" << std::endl;
    }

    void deliver(Order& order) override {
        std::cout << "  [操作] 签收确认" << std::endl;
        order.setState(deliveredState());
    }

    void refund(Order& order) override {
        std::cout << "  [操作] 申请退货退款" << std::endl;
        order.setState(refundingState());
    }

    static std::shared_ptr<ShippedState> instance() {
        static auto inst = std::shared_ptr<ShippedState>(new ShippedState());
        return inst;
    }

private:
    ShippedState() = default;
};

// 具体状态：已签收
class DeliveredState : public IOrderState,
                       public std::enable_shared_from_this<DeliveredState> {
public:
    std::string getName() const override { return "已签收"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：订单已完成" << std::endl;
    }

    void cancel(Order&) override {
        std::cout << "  [操作] 错误：已签收，不能取消" << std::endl;
    }

    void ship(Order&) override {
        std::cout << "  [操作] 错误：订单已完成" << std::endl;
    }

    void deliver(Order&) override {
        std::cout << "  [操作] 错误：已签收" << std::endl;
    }

    void refund(Order& order) override {
        std::cout << "  [操作] 申请售后退款" << std::endl;
        order.setState(refundingState());
    }

    static std::shared_ptr<DeliveredState> instance() {
        static auto inst = std::shared_ptr<DeliveredState>(new DeliveredState());
        return inst;
    }

private:
    DeliveredState() = default;
};

// 具体状态：退款中
class RefundingState : public IOrderState,
                       public std::enable_shared_from_this<RefundingState> {
public:
    std::string getName() const override { return "退款中"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：退款中" << std::endl;
    }

    void cancel(Order&) override {
        std::cout << "  [操作] 错误：退款中" << std::endl;
    }

    void ship(Order&) override {
        std::cout << "  [操作] 错误：退款中" << std::endl;
    }

    void deliver(Order&) override {
        std::cout << "  [操作] 错误：退款中" << std::endl;
    }

    void refund(Order& order) override {
        std::cout << "  [操作] 退款完成" << std::endl;
        order.setState(refundedState());
    }

    static std::shared_ptr<RefundingState> instance() {
        static auto inst = std::shared_ptr<RefundingState>(new RefundingState());
        return inst;
    }

private:
    RefundingState() = default;
};

// 具体状态：已退款（终态）
class RefundedState : public IOrderState,
                      public std::enable_shared_from_this<RefundedState> {
public:
    std::string getName() const override { return "已退款"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：订单已退款关闭" << std::endl;
    }
    void cancel(Order&) override {
        std::cout << "  [操作] 错误：订单已退款关闭" << std::endl;
    }
    void ship(Order&) override {
        std::cout << "  [操作] 错误：订单已退款关闭" << std::endl;
    }
    void deliver(Order&) override {
        std::cout << "  [操作] 错误：订单已退款关闭" << std::endl;
    }
    void refund(Order&) override {
        std::cout << "  [操作] 错误：已退款" << std::endl;
    }

    static std::shared_ptr<RefundedState> instance() {
        static auto inst = std::shared_ptr<RefundedState>(new RefundedState());
        return inst;
    }

private:
    RefundedState() = default;
};

// 具体状态：已取消（终态）
class CancelledState : public IOrderState,
                       public std::enable_shared_from_this<CancelledState> {
public:
    std::string getName() const override { return "已取消"; }

    void pay(Order&) override {
        std::cout << "  [操作] 错误：订单已取消" << std::endl;
    }
    void cancel(Order&) override {
        std::cout << "  [操作] 错误：已取消" << std::endl;
    }
    void ship(Order&) override {
        std::cout << "  [操作] 错误：订单已取消" << std::endl;
    }
    void deliver(Order&) override {
        std::cout << "  [操作] 错误：订单已取消" << std::endl;
    }
    void refund(Order&) override {
        std::cout << "  [操作] 错误：已取消订单无退款" << std::endl;
    }

    static std::shared_ptr<CancelledState> instance() {
        static auto inst = std::shared_ptr<CancelledState>(new CancelledState());
        return inst;
    }

private:
    CancelledState() = default;
};

// 状态工厂定义：复用各状态类的单例实例
std::shared_ptr<IOrderState> paidState() { return PaidState::instance(); }
std::shared_ptr<IOrderState> cancelledState() { return CancelledState::instance(); }
std::shared_ptr<IOrderState> shippedState() { return ShippedState::instance(); }
std::shared_ptr<IOrderState> deliveredState() { return DeliveredState::instance(); }
std::shared_ptr<IOrderState> refundingState() { return RefundingState::instance(); }
std::shared_ptr<IOrderState> refundedState() { return RefundedState::instance(); }

// ============================================================
// 2. 状态转换表（表格驱动状态机）
// ============================================================

/**
 * 对于状态较多、转换规则清晰的场景，可以使用状态转换表。
 * 使用表格数据来描述状态转换，而不是用类的方法。
 *
 * 优点：
 * - 状态和转换一目了然
 * - 容易修改和扩展
 * - 适合状态转换规则可以数据化的场景
 */

// 交通灯状态
enum class TrafficLightState {
    Red,
    Yellow,
    Green,
    FlashingYellow  // 黄灯闪烁（夜间模式）
};

// 交通灯事件
enum class TrafficLightEvent {
    Timer,      // 定时器触发
    NightMode,  // 进入夜间模式
    DayMode     // 进入日间模式
};

// 表格驱动的状态机
class TrafficLightFSM {
public:
    using State = TrafficLightState;
    using Event = TrafficLightEvent;

    TrafficLightFSM() : currentState_(State::Red) {}

    void handleEvent(Event event) {
        auto key = std::make_pair(currentState_, event);
        auto it = transitionTable_.find(key);
        if (it != transitionTable_.end()) {
            State newState = it->second.first;
            auto action = it->second.second;
            std::cout << "  [交通灯] " << stateName(currentState_)
                      << " + " << eventName(event)
                      << " -> " << stateName(newState) << std::endl;
            if (action) action();
            currentState_ = newState;
        } else {
            std::cout << "  [交通灯] " << stateName(currentState_)
                      << " + " << eventName(event)
                      << " -> 无效转换，保持当前状态" << std::endl;
        }
    }

    State getCurrentState() const { return currentState_; }

private:
    State currentState_;

    // 转换表: (当前状态, 事件) -> (下一状态, 动作)
    std::map<std::pair<State, Event>, std::pair<State, std::function<void()>>> transitionTable_ = {
        // 日间正常转换
        {{State::Red, Event::Timer}, {State::Green, [](){ std::cout << "    (放行)" << std::endl; }}},
        {{State::Green, Event::Timer}, {State::Yellow, [](){ std::cout << "    (减速)" << std::endl; }}},
        {{State::Yellow, Event::Timer}, {State::Red, [](){ std::cout << "    (停车)" << std::endl; }}},

        // 进入夜间模式
        {{State::Red, Event::NightMode}, {State::FlashingYellow, [](){ std::cout << "    (进入夜间模式)" << std::endl; }}},
        {{State::Yellow, Event::NightMode}, {State::FlashingYellow, [](){ std::cout << "    (进入夜间模式)" << std::endl; }}},
        {{State::Green, Event::NightMode}, {State::FlashingYellow, [](){ std::cout << "    (进入夜间模式)" << std::endl; }}},

        // 夜间模式定时器（保持闪烁）
        {{State::FlashingYellow, Event::Timer}, {State::FlashingYellow, [](){ std::cout << "    (黄灯闪烁，注意安全)" << std::endl; }}},

        // 退出夜间模式
        {{State::FlashingYellow, Event::DayMode}, {State::Red, [](){ std::cout << "    (恢复日间模式)" << std::endl; }}},
    };

    static std::string stateName(State s) {
        switch (s) {
            case State::Red: return "红灯";
            case State::Yellow: return "黄灯";
            case State::Green: return "绿灯";
            case State::FlashingYellow: return "黄灯闪烁";
        }
        return "未知";
    }

    static std::string eventName(Event e) {
        switch (e) {
            case Event::Timer: return "定时器";
            case Event::NightMode: return "夜间模式";
            case Event::DayMode: return "日间模式";
        }
        return "未知";
    }
};

// ============================================================
// 3. 游戏AI状态机示例
// ============================================================

/**
 * 游戏AI是状态模式的典型应用场景。
 * AI角色有多种状态（巡逻、追击、攻击、逃跑、死亡等），
 * 每种状态下有不同的行为，状态之间根据条件转换。
 */

class GameAI;

// AI状态
class IAIState {
public:
    virtual ~IAIState() = default;
    virtual std::string getName() const = 0;
    virtual void enter(GameAI& ai) = 0;     // 进入状态
    virtual void update(GameAI& ai) = 0;    // 更新状态
    virtual void exit(GameAI& ai) = 0;      // 退出状态

    // 终止状态（死亡）不允许再切换到别的状态
    virtual bool isTerminal() const { return false; }
};

// AI 状态工厂：定义在具体状态类之后
std::shared_ptr<IAIState> patrolState();
std::shared_ptr<IAIState> chaseState();
std::shared_ptr<IAIState> attackState();
std::shared_ptr<IAIState> fleeState();
std::shared_ptr<IAIState> deadState();

// AI角色
class GameAI {
public:
    GameAI(std::string name, int health)
        : name_(std::move(name)), health_(health), maxHealth_(health) {
    }

    void changeState(std::shared_ptr<IAIState> newState) {
        // 已经处于终止状态（死亡）时，不允许再切换到其他状态
        if (currentState_ && currentState_->isTerminal()) {
            return;
        }
        // 血量归零后只允许进入死亡状态
        if (isDead() && !newState->isTerminal()) {
            return;
        }
        if (currentState_) {
            std::cout << "  [AI " << name_ << "] 退出 " << currentState_->getName() << std::endl;
            currentState_->exit(*this);
        }
        currentState_ = std::move(newState);
        std::cout << "  [AI " << name_ << "] 进入 " << currentState_->getName() << std::endl;
        currentState_->enter(*this);
    }

    void update() {
        if (currentState_) {
            currentState_->update(*this);
        }
    }

    // 属性
    const std::string& getName() const { return name_; }
    int getHealth() const { return health_; }
    void takeDamage(int damage) {
        if (isDead()) return;  // 已经死亡，不再重复扣血
        health_ = std::max(0, health_ - damage);
        std::cout << "  [AI " << name_ << "] 受到 " << damage << " 点伤害，剩余HP: "
                  << health_ << "/" << maxHealth_ << std::endl;
        if (health_ == 0) {
            changeState(deadState());  // 血量归零 -> 自动进入死亡状态
        }
    }
    void heal(int amount) {
        if (isDead()) {
            std::cout << "  [AI " << name_ << "] 已死亡，无法恢复HP" << std::endl;
            return;
        }
        health_ = std::min(maxHealth_, health_ + amount);
        std::cout << "  [AI " << name_ << "] 恢复 " << amount << " 点HP，当前HP: "
                  << health_ << "/" << maxHealth_ << std::endl;
    }
    bool isPlayerNearby() const { return playerNearby_; }
    void setPlayerNearby(bool nearby) { playerNearby_ = nearby; }
    bool isLowHealth() const { return health_ < maxHealth_ * 0.3; }
    bool isDead() const { return health_ <= 0; }
    int getAttackPower() const { return 10; }

    std::string getStateName() const {
        return currentState_ ? currentState_->getName() : "无状态";
    }

private:
    std::string name_;
    int health_;
    int maxHealth_;
    bool playerNearby_ = false;
    std::shared_ptr<IAIState> currentState_;
};

// 具体状态：巡逻
class PatrolState : public IAIState {
public:
    std::string getName() const override { return "巡逻状态"; }

    void enter(GameAI&) override {
        std::cout << "    开始在区域内巡逻..." << std::endl;
    }

    void update(GameAI& ai) override {
        // 检查是否死亡
        if (ai.isDead()) return;

        // 发现玩家 -> 追击
        if (ai.isPlayerNearby()) {
            std::cout << "    发现玩家！" << std::endl;
            ai.changeState(chaseState());
            return;
        }

        // 血量低且安全 -> 回血（这里简化为巡逻时慢慢回血）
        if (ai.isLowHealth()) {
            ai.heal(5);
        }

        std::cout << "    巡逻中..." << std::endl;
    }

    void exit(GameAI&) override {
        std::cout << "    停止巡逻" << std::endl;
    }
};

// 具体状态：追击
class ChaseState : public IAIState {
public:
    std::string getName() const override { return "追击状态"; }

    void enter(GameAI&) override {
        std::cout << "    开始追击玩家！" << std::endl;
    }

    void update(GameAI& ai) override {
        // 检查死亡
        if (ai.isDead()) return;

        // 玩家跑了 -> 回到巡逻
        if (!ai.isPlayerNearby()) {
            std::cout << "    失去目标..." << std::endl;
            ai.changeState(patrolState());
            return;
        }

        // 血量低 -> 逃跑
        if (ai.isLowHealth()) {
            std::cout << "    血量过低，撤退！" << std::endl;
            ai.changeState(fleeState());
            return;
        }

        // 追击（模拟）
        chaseCount_++;
        std::cout << "    追击玩家中... (距离: " << std::max(0, 100 - chaseCount_ * 10) << "m)" << std::endl;

        // 追近了 -> 攻击
        if (chaseCount_ >= 3) {
            chaseCount_ = 0;
            ai.changeState(attackState());
        }
    }

    void exit(GameAI&) override {
        chaseCount_ = 0;
    }

private:
    int chaseCount_ = 0;
};

// 具体状态：攻击
class AttackState : public IAIState {
public:
    std::string getName() const override { return "攻击状态"; }

    void enter(GameAI&) override {
        std::cout << "    进入攻击姿态！" << std::endl;
    }

    void update(GameAI& ai) override {
        if (ai.isDead()) return;

        // 玩家跑了 -> 追击
        if (!ai.isPlayerNearby()) {
            ai.changeState(chaseState());
            return;
        }

        // 血量低 -> 逃跑
        if (ai.isLowHealth()) {
            std::cout << "    血量过低，撤退！" << std::endl;
            ai.changeState(fleeState());
            return;
        }

        // 攻击
        std::cout << "    攻击玩家！造成 " << ai.getAttackPower() << " 点伤害" << std::endl;
        attackCount_++;

        // 攻击几次后拉开距离
        if (attackCount_ >= 3) {
            attackCount_ = 0;
            std::cout << "    后撤步..." << std::endl;
            ai.changeState(chaseState());
        }
    }

    void exit(GameAI&) override {
        attackCount_ = 0;
    }

private:
    int attackCount_ = 0;
};

// 具体状态：逃跑
class FleeState : public IAIState {
public:
    std::string getName() const override { return "逃跑状态"; }

    void enter(GameAI&) override {
        std::cout << "    玩命逃跑中！" << std::endl;
    }

    void update(GameAI& ai) override {
        if (ai.isDead()) return;

        fleeCount_++;
        std::cout << "    逃跑中... (已跑 " << fleeCount_ * 20 << "m)" << std::endl;

        // 逃远了 -> 回到巡逻
        if (fleeCount_ >= 5) {
            fleeCount_ = 0;
            ai.setPlayerNearby(false);
            std::cout << "    成功逃脱！" << std::endl;
            ai.changeState(patrolState());
        }
    }

    void exit(GameAI&) override {
        fleeCount_ = 0;
    }

private:
    int fleeCount_ = 0;
};

// 具体状态：死亡
class DeadState : public IAIState {
public:
    std::string getName() const override { return "死亡状态"; }
    bool isTerminal() const override { return true; }

    void enter(GameAI& ai) override {
        std::cout << "    AI " << ai.getName() << " 被击败了！" << std::endl;
    }

    void update(GameAI&) override {
        // 死亡状态不做任何事
    }

    void exit(GameAI&) override {
        // 不会退出死亡状态
    }
};

// AI 状态工厂：每个 AI 拥有独立的状态对象，计数器互不影响
std::shared_ptr<IAIState> patrolState() { return std::make_shared<PatrolState>(); }
std::shared_ptr<IAIState> chaseState() { return std::make_shared<ChaseState>(); }
std::shared_ptr<IAIState> attackState() { return std::make_shared<AttackState>(); }
std::shared_ptr<IAIState> fleeState() { return std::make_shared<FleeState>(); }
std::shared_ptr<IAIState> deadState() { return std::make_shared<DeadState>(); }

// ============================================================
// 测试代码
// ============================================================

void testClassicState() {
    std::cout << "\n========== 经典状态模式（订单状态机）测试 ==========" << std::endl;

    Order order;
    order.setState(PendingPaymentState::instance());

    std::cout << "\n--- 正常流程：支付 -> 发货 -> 签收 ---" << std::endl;
    order.pay();
    order.ship();
    order.deliver();

    // 重新创建一个订单测试取消
    std::cout << "\n--- 取消流程：待支付 -> 取消 ---" << std::endl;
    Order order2;
    order2.setState(PendingPaymentState::instance());
    order2.cancel();

    // 退款流程
    std::cout << "\n--- 退款流程：已支付 -> 退款中 -> 已退款 ---" << std::endl;
    Order order3;
    order3.setState(PaidState::instance());
    order3.refund();
    order3.refund();  // 第二次调用完成退款

    // 非法操作测试
    std::cout << "\n--- 非法操作测试 ---" << std::endl;
    Order order4;
    order4.setState(PendingPaymentState::instance());
    order4.ship();     // 未支付不能发货
    order4.deliver();  // 未支付不能签收

    std::cout << "\n  状态模式将每个状态的行为封装在独立的类中" << std::endl;
    std::cout << "  消除了大量的if/else或switch语句" << std::endl;
}

void testStateTransitionTable() {
    std::cout << "\n========== 状态转换表测试 ==========" << std::endl;

    TrafficLightFSM trafficLight;

    std::cout << "\n--- 日间循环 ---" << std::endl;
    for (int i = 0; i < 6; ++i) {
        trafficLight.handleEvent(TrafficLightFSM::Event::Timer);
    }

    std::cout << "\n--- 进入夜间模式 ---" << std::endl;
    trafficLight.handleEvent(TrafficLightFSM::Event::NightMode);
    for (int i = 0; i < 3; ++i) {
        trafficLight.handleEvent(TrafficLightFSM::Event::Timer);
    }

    std::cout << "\n--- 恢复日间模式 ---" << std::endl;
    trafficLight.handleEvent(TrafficLightFSM::Event::DayMode);
    trafficLight.handleEvent(TrafficLightFSM::Event::Timer);

    std::cout << "\n  表格驱动状态机的优势：" << std::endl;
    std::cout << "  - 状态转换规则一目了然" << std::endl;
    std::cout << "  - 易于维护和扩展" << std::endl;
    std::cout << "  - 适合规则可以数据化的场景" << std::endl;
}

void testGameAI() {
    std::cout << "\n========== 游戏AI状态机测试 ==========" << std::endl;

    GameAI enemy("哥布林", 100);
    enemy.changeState(patrolState());

    std::cout << "\n--- 第1回合：巡逻 ---" << std::endl;
    enemy.update();

    std::cout << "\n--- 第2回合：玩家出现，开始追击 ---" << std::endl;
    enemy.setPlayerNearby(true);
    enemy.update();  // 发现玩家 -> 追击

    std::cout << "\n--- 第3-5回合：追击 -> 攻击 ---" << std::endl;
    enemy.update();  // 追击1
    enemy.update();  // 追击2
    enemy.update();  // 追击3 -> 攻击

    std::cout << "\n--- 第6-8回合：攻击 ---" << std::endl;
    enemy.update();  // 攻击1
    enemy.update();  // 攻击2
    enemy.update();  // 攻击3 -> 追击

    std::cout << "\n--- 第9回合：受到大量伤害，逃跑 ---" << std::endl;
    enemy.takeDamage(80);  // 打到20血，低于30%
    enemy.update();  // 发现血量低 -> 逃跑

    std::cout << "\n--- 第10-14回合：逃跑中 ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        enemy.update();
    }

    std::cout << "\n--- 最终状态 ---" << std::endl;
    std::cout << "  AI状态: " << enemy.getStateName() << std::endl;
    std::cout << "  AI血量: " << enemy.getHealth() << "/100" << std::endl;

    std::cout << "\n--- 第15回合：致命一击，进入死亡状态 ---" << std::endl;
    enemy.takeDamage(100);  // 血量归零 -> 自动进入死亡状态
    enemy.update();         // 死亡状态不再行动
    enemy.setPlayerNearby(true);
    enemy.update();         // 即使玩家靠近，也不会离开死亡状态
    enemy.heal(50);         // 死亡后不能回血
    std::cout << "  AI状态: " << enemy.getStateName() << std::endl;
    std::cout << "  AI血量: " << enemy.getHealth() << "/100" << std::endl;

    std::cout << "\n  游戏AI状态机展示了状态模式的典型应用" << std::endl;
    std::cout << "  每个状态封装了进入/更新/退出的行为" << std::endl;
    std::cout << "  状态转换条件清晰，易于扩展新状态" << std::endl;
    std::cout << "  死亡是终止状态，进入后不能再切换到其他状态" << std::endl;
}

int main() {
    std::cout << "========== 状态模式学习 Demo ==========" << std::endl;

    testClassicState();
    testStateTransitionTable();
    testGameAI();

    std::cout << "\n========== 状态模式总结 ==========" << std::endl;
    std::cout << "核心思想：对象内部状态改变时，行为也随之改变" << std::endl;
    std::cout << "经典状态：每个状态一个类，封装行为和转换逻辑" << std::endl;
    std::cout << "状态转换表：表格驱动，规则清晰，易于维护" << std::endl;
    std::cout << "游戏AI：状态模式的典型应用，enter/update/exit三方法" << std::endl;
    std::cout << "vs策略模式：状态有转换关系，策略平等独立" << std::endl;
    std::cout << "设计原则：状态行为局部化（单一职责）；开闭原则仅部分满足" << std::endl;

    return 0;
}
