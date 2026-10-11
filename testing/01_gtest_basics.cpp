// ============================================================
// 01_gtest_basics.cpp —— Google Test 入门
// ============================================================
//
// 编译命令（需要先安装 Google Test）：
//   g++ -std=c++17 01_gtest_basics.cpp -lgtest -lgtest_main -lpthread -o demo
//
// 安装 GTest（Ubuntu）：
//   sudo apt-get install libgtest-dev
//   cd /usr/src/gtest && sudo cmake . && sudo make && sudo cp lib/*.a /usr/lib
//
// ============================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================
// 被测对象：StringUtils 工具类
// ============================================================
// 我们用一个简单的字符串工具类来演示单元测试的各种写法。
// 写测试的时候，你就把自己当成"质检员"，想尽办法让代码出错。
class StringUtils {
public:
    // 判断字符串是否为空
    static bool isEmpty(const std::string& s) {
        return s.empty();
    }

    // 字符串反转
    static std::string reverse(const std::string& s) {
        std::string result = s;
        std::reverse(result.begin(), result.end());
        return result;
    }

    // 字符串转大写
    static std::string toUpper(const std::string& s) {
        std::string result = s;
        for (char& c : result) {
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }
        }
        return result;
    }

    // 判断是否是回文字符串（正读反读都一样）
    static bool isPalindrome(const std::string& s) {
        int left = 0;
        int right = static_cast<int>(s.size()) - 1;
        while (left < right) {
            if (s[left] != s[right]) return false;
            left++;
            right--;
        }
        return true;
    }

    // 字符串拼接
    static std::string concat(const std::string& a, const std::string& b) {
        return a + b;
    }
};

// ============================================================
// 一、TEST 宏：最简单的测试用例
// ============================================================
// TEST(测试套件名, 测试用例名)
// - 测试套件（Test Suite）：一组相关的测试用例
// - 测试用例（Test Case）：一个具体的测试场景
//
// 命名建议：
//   测试套件名 = 被测类/模块名
//   测试用例名 = 功能描述 + 预期结果
//
// 注意：GTest 的测试用例是按字母顺序执行的，不要依赖执行顺序！

// 测试套件：StringUtilsTest
// 测试用例：测试 isEmpty 函数
TEST(StringUtilsTest, IsEmptyReturnsTrueForEmptyString) {
    // 布尔断言：ASSERT_TRUE / ASSERT_FALSE
    // ASSERT_* 系列：失败则立即终止当前测试用例
    ASSERT_TRUE(StringUtils::isEmpty(""));
    ASSERT_FALSE(StringUtils::isEmpty("hello"));
}

TEST(StringUtilsTest, ReverseWorksCorrectly) {
    // 字符串断言：ASSERT_STREQ / ASSERT_STRNE
    // 用于比较 C 风格字符串（const char*）
    std::string result = StringUtils::reverse("abcde");
    ASSERT_STREQ(result.c_str(), "edcba");
}

TEST(StringUtilsTest, ToUpperConvertsLowercase) {
    // 数值/对象比较：ASSERT_EQ / ASSERT_NE
    // EQ = equal, NE = not equal
    ASSERT_EQ(StringUtils::toUpper("hello"), "HELLO");
    ASSERT_EQ(StringUtils::toUpper("HeLLo"), "HELLO");
    ASSERT_EQ(StringUtils::toUpper("123"), "123");  // 数字不变
}

// ============================================================
// 二、ASSERT_* vs EXPECT_* 的区别
// ============================================================
// ASSERT_*：失败后立即终止当前测试函数（致命错误）
// EXPECT_*：失败后继续执行当前测试函数（非致命错误）
//
// 什么时候用哪个？
// - 如果后续测试依赖这个条件，用 ASSERT（比如指针非空判断）
// - 如果是独立的检查，用 EXPECT（可以一次看到所有失败点）

TEST(AssertVsExpectTest, DemonstrateDifference) {
    int a = 1, b = 2, c = 3;

    // EXPECT 失败后继续执行
    EXPECT_EQ(a, b);   // 会失败，但后面的代码还会跑
    EXPECT_EQ(b, c);   // 也会失败
    EXPECT_EQ(a, 1);   // 这个通过

    // 对比：如果用 ASSERT，第一个失败后后面都不执行了
    // 你可以把上面的 EXPECT 改成 ASSERT 试试效果
}

// ============================================================
// 三、各种断言类型一览
// ============================================================

TEST(AssertionTypesTest, BooleanAssertions) {
    // 布尔断言
    ASSERT_TRUE(1 + 1 == 2);
    ASSERT_FALSE(1 + 1 == 3);
}

TEST(AssertionTypesTest, NumericAssertions) {
    // 数值断言
    ASSERT_EQ(1 + 1, 2);     // 等于
    ASSERT_NE(1 + 1, 3);     // 不等于
    ASSERT_LT(1, 2);         // 小于 (Less Than)
    ASSERT_LE(2, 2);         // 小于等于 (Less or Equal)
    ASSERT_GT(3, 2);         // 大于 (Greater Than)
    ASSERT_GE(3, 3);         // 大于等于 (Greater or Equal)
}

TEST(AssertionTypesTest, StringAssertions) {
    // 字符串断言（针对 const char*）
    // 注意：比较 std::string 用 ASSERT_EQ 即可
    const char* s1 = "hello";
    const char* s2 = "hello";
    const char* s3 = "world";

    ASSERT_STREQ(s1, s2);     // 字符串相等
    ASSERT_STRNE(s1, s3);     // 字符串不等
    ASSERT_STRCASEEQ("HELLO", "hello");  // 忽略大小写相等
    ASSERT_STRCASENE("HELLO", "world");  // 忽略大小写不等
}

TEST(AssertionTypesTest, FloatAssertions) {
    // 浮点断言
    // 浮点数不能直接用 == 比较，因为有精度误差
    double a = 0.1 + 0.2;
    double b = 0.3;

    // 直接比较可能失败（因为浮点数精度问题）
    // ASSERT_EQ(a, b);  // 这行大概率会失败！

    // 用浮点专用断言
    ASSERT_FLOAT_EQ(0.1f + 0.2f, 0.3f);   // float 精度比较
    ASSERT_DOUBLE_EQ(a, b);               // double 精度比较

    // 也可以指定误差范围
    ASSERT_NEAR(a, b, 1e-10);  // 两个值的差不超过 1e-10
}

// ============================================================
// 四、TEST_F 测试夹具（Test Fixture）
// ============================================================
// 当多个测试用例需要相同的准备/清理工作时，用测试夹具。
// 就像做饭前先摆好碗筷（SetUp），吃完再收拾（TearDown）。
//
// 使用步骤：
//   1. 定义一个类，继承 public ::testing::Test
//   2. 在类里定义成员变量（被测对象）
//   3. 重写 SetUp() —— 每个测试用例执行前调用
//   4. 重写 TearDown() —— 每个测试用例执行后调用
//   5. 用 TEST_F(类名, 用例名) 写测试

class StringUtilsFixture : public ::testing::Test {
protected:
    // 每个测试用例执行前都会调用 SetUp
    void SetUp() override {
        // 可以在这里做初始化工作
        // 比如打开文件、连接数据库、创建对象等
        emptyStr = "";
        helloStr = "hello";
        palindromeStr = "abcba";
    }

    // 每个测试用例执行后都会调用 TearDown
    void TearDown() override {
        // 在这里做清理工作
        // 比如释放资源、关闭文件等
    }

    // 成员变量：所有 TEST_F 都能直接访问
    std::string emptyStr;
    std::string helloStr;
    std::string palindromeStr;
};

// 注意：第一个参数必须是夹具类名
TEST_F(StringUtilsFixture, IsEmptyTest) {
    // 可以直接使用夹具里的成员变量
    ASSERT_TRUE(StringUtils::isEmpty(emptyStr));
    ASSERT_FALSE(StringUtils::isEmpty(helloStr));
}

TEST_F(StringUtilsFixture, PalindromeTest) {
    ASSERT_TRUE(StringUtils::isPalindrome(palindromeStr));
    ASSERT_FALSE(StringUtils::isPalindrome(helloStr));
    ASSERT_TRUE(StringUtils::isPalindrome(emptyStr));  // 空串也是回文
}

TEST_F(StringUtilsFixture, ConcatTest) {
    std::string result = StringUtils::concat(helloStr, " world");
    ASSERT_EQ(result, "hello world");
    ASSERT_EQ(result.size(), 11u);  // u 表示 unsigned，消除警告
}

// ============================================================
// 五、测试失败时输出自定义信息
// ============================================================
// 有时候默认的错误信息不够，你可以用 << 追加自己的说明。

TEST(CustomMessageTest, ShowCustomInfoOnFailure) {
    int x = 5;
    int y = 10;

    // 失败时会输出你写的信息，方便排查
    EXPECT_GT(x, y) << "x = " << x << ", y = " << y
                    << ", 期望 x 大于 y，但实际上 x 更小";
}

// ============================================================
// 六、main 函数
// ============================================================
// 如果你链接了 gtest_main（编译命令里的 -lgtest_main），
// 就不需要自己写 main 了，GTest 会帮你处理。
//
// 如果你想自己控制 main 函数的逻辑，可以这样写：
//
//   int main(int argc, char **argv) {
//       ::testing::InitGoogleTest(&argc, argv);
//       return RUN_ALL_TESTS();
//   }
//
// 但本文件用的是 -lgtest_main，所以不用写 main。

// ============================================================
// 老师傅的话
// ============================================================
//
// 小年轻，看到这儿说明你已经把测试基础过了一遍。
// 老师傅我跟你掏心窝子说几句：
//
// 【为什么要写单元测试？】
// 很多新手觉得"写测试是浪费时间"，"我手动测一下不行吗"？
// 行，当然行——代码只有 10 行的时候确实行。
// 但等项目大了，函数有几百个，你改了一个地方，
// 你敢说你记得所有受影响的地方吗？你敢随便重构吗？
//
// 单元测试就是你的"安全网"：
//   1. 改代码不怕搞坏别的功能——跑一遍测试就知道
//   2. 重构有底气——有测试兜底，大刀阔斧改就是
//   3. 文档作用——看测试用例就知道函数怎么用
//   4. 倒逼设计——难测的代码通常设计也不咋地
//
// 【写测试的几个原则】
//   1. 每个测试只测一件事——失败了你一眼就知道哪错了
//   2. 测试之间互相独立——不要 A 测试依赖 B 测试的结果
//   3. 测试要快——慢了大家就不想跑了
//   4. 给测试起个好名字——看到名字就知道测啥，不用看代码
//
// 【常见误区】
//   - 误区1：测试覆盖率 100% 就完事了
//     覆盖率只是个参考数字，测到了不代表测对了。
//     边界条件、异常情况才是 bug 的重灾区。
//
//   - 误区2：只测正常情况
//     好的测试工程师，满脑子都是"怎么让代码崩"。
//     空指针、空字符串、负数、超大值、重复数据……
//     这些边界情况才是你需要重点测的。
//
//   - 误区3：测试代码不用讲究
//     测试代码也是代码，也要可读性、可维护性。
//     不然哪天业务改了，测试代码先烂掉了。
//
// 记住：测试不是为了证明代码"没问题"，
// 而是为了让你有信心说"我知道哪里可能有问题"。
//
// ============================================================
// 小练习
// ============================================================
//
// 1. 给 StringUtils 加一个函数：int countChar(const string& s, char c)
//    功能是统计字符串中某个字符出现的次数。
//    然后为这个函数写 5 个以上的测试用例，覆盖各种边界情况。
//
// 2. 写一个简单的栈类（MyStack），支持 push/pop/top/empty/size。
//    然后用 TEST_F 写测试夹具，每个测试用例前创建一个空栈。
//    测试场景包括：空栈 pop 是否报错、push 后 size 是否正确等。
//
// 3. 思考：如果一个函数有随机行为（比如生成随机数），怎么测？
//    （提示：可以从"输入输出的约束"入手，而不是具体值）
//
// ============================================================
