// ============================================================
// 02_gtest_advanced.cpp —— Google Test 进阶
// ============================================================
//
// 编译命令：
//   g++ -std=c++17 02_gtest_advanced.cpp -lgtest -lgtest_main -lpthread -o demo
//
// 注意：本文件有 2 个“故意失败”的用例，用于演示 SCOPED_TRACE 的定位信息：
//   ScopedTraceTest.HelpsDebugLoopFailures
//   ScopedTraceTest.WorksInHelperFunctions
// 跑完整套件时看到 2 个失败是预期行为，不是代码坏了。
// 跳过它们：
//   demo --gtest_filter=-ScopedTraceTest.*
//
// ============================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <tuple>
#include <climits>

// ============================================================
// 被测对象 1：计算器类
// ============================================================
class Calculator {
public:
    int add(int a, int b) const { return a + b; }
    int subtract(int a, int b) const { return a - b; }
    int multiply(int a, int b) const { return a * b; }

    // 除法：除数为 0 时抛出异常
    double divide(double a, double b) const {
        if (b == 0) {
            throw std::invalid_argument("除数不能为零");
        }
        return a / b;
    }

    // 阶乘：负数时直接 abort（用于演示死亡测试）
    int factorial(int n) const {
        if (n < 0) {
            // 直接终止程序（模拟致命错误）
            std::abort();
        }
        int result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }
};

// ============================================================
// 被测对象 2：排序函数（冒泡排序）
// ============================================================
void bubbleSort(std::vector<int>& arr) {
    int n = static_cast<int>(arr.size());
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// ============================================================
// 一、参数化测试（TEST_P）
// ============================================================
// 当你想用多组输入输出测试同一个逻辑时，参数化测试是你的好帮手。
// 不用一个一个写 TEST，一组参数对应一个测试用例。
//
// 使用步骤：
//   1. 定义夹具类，继承 public ::testing::TestWithParam<T>
//   2. 用 TEST_P 写测试逻辑，通过 GetParam() 获取参数
//   3. 用 INSTANTIATE_TEST_SUITE_P 注册参数列表

// --- 示例 1：计算器加法的参数化测试 ---

// 夹具类：参数类型是 std::tuple<int, int, int>（a, b, 期望结果）
class AddTest : public ::testing::TestWithParam<std::tuple<int, int, int>> {
protected:
    Calculator calc;
};

// TEST_P 里写测试逻辑
TEST_P(AddTest, ReturnsCorrectSum) {
    int a = std::get<0>(GetParam());
    int b = std::get<1>(GetParam());
    int expected = std::get<2>(GetParam());

    EXPECT_EQ(calc.add(a, b), expected)
        << "a = " << a << ", b = " << b << ", 期望 = " << expected;
}

// 注册参数：每组参数都会生成一个独立的测试用例
INSTANTIATE_TEST_SUITE_P(
    AddTestCases,           // 测试套件前缀名
    AddTest,                // 夹具类名
    ::testing::Values(      // 参数列表
        std::make_tuple(1, 1, 2),        // 正常正数
        std::make_tuple(0, 0, 0),        // 零
        std::make_tuple(-1, -1, -2),     // 负数
        std::make_tuple(100, -50, 50),   // 正负混合
        std::make_tuple(0, 123, 123),    // 零加正数
        std::make_tuple(INT_MAX, 0, INT_MAX)  // 边界值
    )
);

// --- 示例 2：排序函数的参数化测试 ---
// 用结构体来组织参数，可读性更好

struct SortTestCase {
    std::vector<int> input;
    std::vector<int> expected;
    std::string description;  // 用例描述，方便看哪个失败了
};

class SortTest : public ::testing::TestWithParam<SortTestCase> {
};

TEST_P(SortTest, SortsCorrectly) {
    const auto& tc = GetParam();
    std::vector<int> arr = tc.input;  // 复制一份，不修改原数据
    bubbleSort(arr);
    EXPECT_EQ(arr, tc.expected) << "失败用例：" << tc.description;
}

// 参数列表
SortTestCase sortCases[] = {
    {{}, {}, "空数组"},
    {{5}, {5}, "单元素"},
    {{1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}, "已排序"},
    {{5, 4, 3, 2, 1}, {1, 2, 3, 4, 5}, "完全逆序"},
    {{3, 1, 4, 1, 5, 9, 2, 6}, {1, 1, 2, 3, 4, 5, 6, 9}, "随机乱序"},
    {{2, 2, 2, 2}, {2, 2, 2, 2}, "全部相同"},
};

INSTANTIATE_TEST_SUITE_P(
    SortTestCases,
    SortTest,
    ::testing::ValuesIn(sortCases)  // 用 ValuesIn 从数组/容器读取
);

// ============================================================
// 二、死亡测试（Death Test）
// ============================================================
// 用来测试"程序应该崩溃/退出"的场景。
// 比如：断言失败、abort、exit 等。
//
// 宏：
//   ASSERT_DEATH(语句, "预期的错误信息正则")
//   ASSERT_EXIT(语句, 退出条件, "预期输出")
//
// 注意：死亡测试在 POSIX 上通常 fork 子进程来跑，Windows 上使用
// 另一套机制（CreateProcess 等），两者都比较慢，且行为有平台差异。
//       通常只在确实需要测"程序会死"的场景才用。

class DeathTest : public ::testing::Test {
protected:
    Calculator calc;
};

TEST_F(DeathTest, FactorialOfNegativeAborts) {
    // 负数阶乘应该直接 abort
    // 第二个参数是正则表达式，匹配错误输出
    // 这里用 ".*" 表示匹配任意输出（因为 abort 的输出可能不一样）
    ASSERT_DEATH(calc.factorial(-1), ".*");
}

// 测试 exit 退出码
TEST_F(DeathTest, ExitWithCode) {
    // ASSERT_EXIT 可以检查退出码
    // 第二个参数是退出条件：ExitedWithCode(code) 表示正常退出且返回码为 code
    ASSERT_EXIT(
        std::exit(42),
        ::testing::ExitedWithCode(42),
        ".*"
    );
}

// ============================================================
// 三、SCOPED_TRACE 追踪
// ============================================================
// 当测试失败时，GTest 会告诉你哪一行失败了。
// 但如果你的测试用例里有循环或者调用了辅助函数，
// 你可能想知道"是第几次循环失败的"或者"调用链是怎样的"。
//
// SCOPED_TRACE("描述信息") 会在当前作用域内添加一个追踪标记，
// 一旦这个作用域里有断言失败，就会把标记的信息一起打印出来。

TEST(ScopedTraceTest, HelpsDebugLoopFailures) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (size_t i = 0; i < data.size(); i++) {
        // 每次循环加一个追踪标记
        SCOPED_TRACE(std::string("第 ") + std::to_string(i) + " 次循环");
        EXPECT_LT(data[i], 7);  // 到第 7 个元素（值为 7）时会失败
    }
    // 失败时你会看到类似这样的输出：
    //   第 6 次循环
    //   Expected: data[i] < 7
    //   Actual: 7 vs 7
}

// 辅助函数里也能用
void checkPositive(int x) {
    SCOPED_TRACE(std::string("checkPositive(") + std::to_string(x) + ")");
    EXPECT_GT(x, 0);
}

TEST(ScopedTraceTest, WorksInHelperFunctions) {
    checkPositive(5);   // 通过
    checkPositive(0);   // 失败，会显示 checkPositive(0)
    checkPositive(-3);  // 失败，会显示 checkPositive(-3)
}

// ============================================================
// 四、测试过滤
// ============================================================
// GTest 支持只跑一部分测试用例，不用全跑。
//
// 方式 1：命令行参数
//   ./demo --gtest_filter=*Add*      只跑名字里有 Add 的测试
//   ./demo --gtest_filter=Sort*      只跑 Sort 开头的测试
//   ./demo --gtest_filter=-*Death*   排除 Death 相关的测试
//
// 方式 2：代码里设置（不推荐，除非特殊需求）
//   ::testing::GTEST_FLAG(filter) = "AddTestCases/*";
//
// 通配符：
//   ?  匹配任意单个字符
//   *  匹配任意多个字符
//   :  分隔多个模式（或的关系）
//   -  排除后面的模式
//
// 例子：
//   --gtest_filter="*Test.*:*.Death*"
//   意思是：跑名字匹配 *Test.* 或者 *.Death* 的测试
//
// 我们这里用一个 TEST 来演示如何在代码里读取过滤标志。

TEST(FilterDemoTest, CheckFilterFlag) {
    // 读取当前的过滤设置（默认是空字符串，表示全跑）
    std::string filter = ::testing::GTEST_FLAG(filter);
    // 这个测试只是演示，不做任何断言
    // 你可以在命令行加 --gtest_filter=*Filter* 试试
    SUCCEED();  // 显式标记成功（可选）
}

// ============================================================
// 五、Mock 概念入门（用纯虚函数演示）
// ============================================================
// Mock（模拟对象）是单元测试里的重要概念。
// 当你的被测对象依赖外部服务（数据库、网络、文件系统）时，
// 你不想真的去连数据库（慢、不稳定、影响数据），
// 这时候就用 Mock 对象来"假装"是那个依赖。
//
// 这里我们不用 Google Mock（gmock）库，
// 只用纯虚函数 + 手动实现来演示 Mock 的思想。
// 等你理解了思想，再学 gmock 就容易了。

// --- 依赖的接口（纯虚基类） ---
class Database {
public:
    virtual ~Database() = default;
    virtual int getUserAge(const std::string& username) = 0;  // 查询用户年龄
    virtual bool connect() = 0;                               // 连接数据库
};

// --- 被测对象：用户服务 ---
// UserService 依赖 Database，但我们不想在测试里真的连数据库
class UserService {
public:
    explicit UserService(Database* db) : db_(db) {}

    // 判断用户是否成年
    bool isAdult(const std::string& username) {
        int age = db_->getUserAge(username);
        return age >= 18;
    }

private:
    Database* db_;
};

// --- Mock 对象：模拟数据库 ---
// 手动实现一个假的数据库，返回我们预设的值
class MockDatabase : public Database {
public:
    // 预设的返回值（测试里可以随便设）
    int fakeAge = 0;
    bool connectCalled = false;

    int getUserAge(const std::string& username) override {
        // 真实的数据库会查表，Mock 直接返回预设值
        return fakeAge;
    }

    bool connect() override {
        connectCalled = true;
        return true;
    }
};

// --- 用 Mock 测 UserService ---
TEST(MockDemoTest, IsAdultReturnsTrueForAdult) {
    MockDatabase mockDb;
    mockDb.fakeAge = 25;  // 预设：返回年龄 25

    UserService service(&mockDb);
    EXPECT_TRUE(service.isAdult("zhangsan"));
    // 注意：我们根本没连真实数据库！
    // Mock 让测试又快又稳定。
}

TEST(MockDemoTest, IsAdultReturnsFalseForMinor) {
    MockDatabase mockDb;
    mockDb.fakeAge = 15;  // 预设：返回年龄 15

    UserService service(&mockDb);
    EXPECT_FALSE(service.isAdult("lisi"));
}

// 老师傅点评：
// 真实的 gmock 库可以做得更强大：
// -  EXPECT_CALL(mock, func(_)).Times(3).WillOnce(Return(42))
// 可以指定函数被调用几次、每次返回什么、参数应该是什么等等。
// 但核心思想是一样的：用假的依赖替换真的依赖，控制测试环境。

// ============================================================
// 六、TDD 基本思路演示
// ============================================================
// TDD = Test-Driven Development（测试驱动开发）
// 核心流程：红 → 绿 → 重构
//   红：先写测试，测试跑不过（红色失败）
//   绿：写最少的代码让测试通过（绿色通过）
//   重构：优化代码，保证测试还是绿的
//
// 下面用一个"斐波那契数列"的例子来走一遍 TDD 流程。
//
// （当然，在一个文件里演示 TDD 有点怪，因为代码和测试都写好了。
//  你要做的是想象这个过程：先有测试，再补实现。）

// 先有测试（TDD 第一步：写测试）
// 我们先想清楚需求：
//   fib(0) = 0
//   fib(1) = 1
//   fib(n) = fib(n-1) + fib(n-2)  （n >= 2）

// 然后写实现（TDD 第二步：写代码让测试通过）
int fib(int n) {
    if (n < 0) return -1;  // 负数返回 -1（错误）
    if (n == 0) return 0;
    if (n == 1) return 1;

    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int c = a + b;
        a = b;
        b = c;
    }
    return b;
}

// 测试用例（TDD 里这些测试应该先写）
TEST(TDDDemoTest, FibonacciBaseCases) {
    EXPECT_EQ(fib(0), 0);
    EXPECT_EQ(fib(1), 1);
}

TEST(TDDDemoTest, FibonacciNormalCases) {
    EXPECT_EQ(fib(2), 1);
    EXPECT_EQ(fib(5), 5);
    EXPECT_EQ(fib(10), 55);
}

TEST(TDDDemoTest, FibonacciNegativeInput) {
    EXPECT_EQ(fib(-1), -1);
    EXPECT_EQ(fib(-100), -1);
}

// 老师傅点评：
// TDD 不是神药，但它确实能逼你想清楚需求再写代码。
// 很多人写代码的毛病是"先写了再说"，写着写着需求变了，
// 改来改去，最后代码一团糟。
// TDD 让你先站在"使用者"的角度想：这个函数应该怎么用？
// 这样设计出来的接口通常更合理。
//
// 要不要学 TDD？要的，但不用强求每一行代码都 TDD。
// 关键是理解"先想清楚再动手"的思想。

// ============================================================
// 老师傅的话
// ============================================================
//
// 小伙子/小姑娘，进阶内容也看完了，不容易啊。
// 老师傅我再跟你唠两句。
//
// 【参数化测试用不用？】
// 能用就用。同样的逻辑，10 组数据就写 10 个 TEST？
// 太蠢了，参数化一行搞定，还清晰。
// 数据驱动测试（Data-Driven Testing）是测试里的重要思想。
//
// 【死亡测试实用吗？】
// 说实话，一般业务代码里用得不多。
// 但如果你写的是底层库、框架代码，
// 要保证"非法输入时程序正确地崩溃"（而不是产生未定义行为），
// 那死亡测试就很有用了。
// 知道有这么个东西就行，需要的时候能想起来。
//
// 【Mock 重要吗？】
// 非常重要。不会 Mock，你写的单元测试永远是"半吊子"。
// 一碰到数据库、网络、文件系统就傻眼了。
// 这个文件里只演示了最基本的 Mock 思想，
// 等你把这个吃透了，去学 Google Mock（gmock）。
// gmock 一开始语法看着吓人，其实核心就那几个东西：
// EXPECT_CALL、Times、WillOnce、WillRepeatedly、Return……
// 多用几次就熟了。
//
// 【测试不是越多越好】
// 新手容易走极端：要么一个测试都不写，要么为了覆盖率瞎写一堆。
// 好的测试应该有"信息量"——跑过了能让你安心，失败了能告诉你问题。
// 如果一个测试永远不会失败，那它毫无价值。
// 如果一个测试的名字叫 test1、test2，那还不如不写。
//
// 【关于 TDD】
// 很多人争论 TDD 好不好，其实没什么好争的。
// TDD 是一种工作方式，适合某些场景某些人，不是万能的。
// 但"测试先行"的思想值得你了解。
// 哪怕你不完全按 TDD 来，
// 写代码之前先想想"这个东西怎么测"，
// 你的设计也会好很多。
//
// 测试这东西，说一千道一万，不如你自己动手写几个。
// 写过一次，你就知道它的价值了。
//
// ============================================================
// 小练习
// ============================================================
//
// 1. 参数化测试练习：
//    给 Calculator 的 divide 函数写参数化测试，
//    覆盖：正常除法、整除、负数、小数结果等场景。
//    除数为 0 的情况用 EXPECT_THROW 来测（自己查一下怎么用）。
//
// 2. Mock 练习：
//    设计一个 Logger 接口（纯虚基类），有 log(message) 方法。
//    再写一个 MockLogger 实现，记录调用次数和最后一条消息。
//    然后写一个 BusinessService 类，它依赖 Logger，
//    当执行 doWork() 时会调用一次 log。
//    用 MockLogger 测试 BusinessService 的 doWork 是否正确调用了日志。
//
// 3. TDD 练习（选做，有挑战性）：
//    尝试用 TDD 的方式写一个"罗马数字转整数"的函数。
//    步骤：
//      1) 先写最简单的测试：romanToInt("I") == 1
//      2) 写最简单的实现让它通过（直接 return 1）
//      3) 再加测试：romanToInt("II") == 2
//      4) 改实现让它通过
//      5) 继续加："III"、"IV"、"IX"、"LVIII"、"MCMXCIV"……
//      6) 每次只加一点点，让测试驱动你完善实现
//    感受一下 TDD 的节奏。
//
// 4. 思考题：
//    什么时候你觉得"写单元测试不值得"？
//    （提示：可以从代码生命周期、团队规模、代码类型等角度想）
//
// ============================================================
