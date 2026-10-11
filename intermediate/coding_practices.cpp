// ============================================================
//  coding_practices.cpp - 编程规范与最佳实践
// ============================================================
//  徒弟啊，技术能力决定你能写多少代码，
//  代码质量决定你能走多远。
//  今天咱聊聊怎么写好代码——命名、格式、注释、接口设计、
//  异常安全、const正确性……这些都是基本功，也是真功夫。
//  记住：代码是写给人看的，顺便给机器执行。
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <algorithm>

using namespace std;

// ============================================================
// 一、命名规范
// ============================================================
// 命名是编程的基本功，也是最能看出水平的地方。
// 好的命名让人一看就懂，坏的命名让人摸不着头脑。
//
// 常见命名风格：
//   - 小驼峰（camelCase）：myVariableName
//   - 大驼峰（PascalCase）：MyClassName
//   - 下划线（snake_case）：my_variable_name
//
// C++常见约定（本项目采用）：
//   - 类名、结构体名、枚举名：大驼峰（MyClass）
//   - 函数名、变量名：小驼峰或下划线（myFunc / my_func）
//   - 成员变量：m_前缀 + 小驼峰（m_count）
//   - 常量：全大写 + 下划线（MAX_SIZE）
//   - 宏定义：全大写 + 下划线（DEBUG_MODE）
//   - 命名空间：全小写（mylib）
//   - 模板参数：单个大写字母（T）或描述性大驼峰（ElementType）
//
// 命名原则：
//   1. 见名知意，别用拼音，别用a/b/c/x/y
//   2. 名字长度适中，太长太啰嗦，太短看不懂
//   3. 风格统一，整个项目保持一致
//   4. 布尔变量用 is/has/can/should 开头
//   5. 函数名用动词开头（get/set/calculate/process）
// ============================================================

// ---- 好的命名示例 ----

// 类名：大驼峰，名词
class UserAccount
{
public:
    // 成员变量：m_前缀 + 小驼峰
    string m_username;
    int m_age;
    bool m_isActive;

    // 函数名：动词开头，小驼峰
    void setUsername(const string& name) { m_username = name; }
    const string& getUsername() const { return m_username; }

    // 布尔函数：is/has/can/should
    bool isAdult() const { return m_age >= 18; }
    bool hasValidName() const { return !m_username.empty(); }
};

// 常量：全大写 + 下划线
const int MAX_RETRY_COUNT = 3;
const double PI = 3.1415926;

// 枚举：类名大驼峰，值全大写
enum class ErrorCode
{
    SUCCESS = 0,
    NOT_FOUND = 1,
    PERMISSION_DENIED = 2,
    INVALID_PARAM = 3
};

// 普通变量：见名知意
void goodNamingExample()
{
    cout << "=== 1. 命名规范 ===" << endl;

    // 好的命名
    int userCount = 0;          // 见名知意
    bool isReady = false;       // 布尔变量用is开头
    double averageScore = 0.0;  // 描述清楚是什么的平均
    vector<string> studentNames; // 复数表示集合

    cout << "  好的命名示例：" << endl;
    cout << "    userCount, isReady, averageScore, studentNames" << endl;

    // 坏的命名（千万别这么写！）
    // int a;           // 什么意思？
    // int n;           // 是数量？还是什么？
    // int flag;        // 什么flag？干啥用的？
    // char* p;         // 指向啥？
    // int tmp;         // 临时变量也得有意义！

    cout << "\n  坏的命名示例（反面教材）：" << endl;
    cout << "    a, n, flag, p, tmp, data, info" << endl;

    cout << "\n  命名原则：" << endl;
    cout << "    1. 见名知意，别让别人猜" << endl;
    cout << "    2. 布尔值用 is/has/can/should 开头" << endl;
    cout << "    3. 函数用动词开头（get/set/calc/check）" << endl;
    cout << "    4. 集合用复数（users, items, values）" << endl;
    cout << "    5. 风格统一，别一会儿驼峰一会儿下划线" << endl;
    cout << "    6. 别用拼音，更别用拼音缩写" << endl;
}

// ============================================================
// 二、代码格式
// ============================================================
// 格式不影响运行，但影响阅读。
// 代码首先是写给人看的，其次才是给机器执行的。
//
// 基本规则：
//   - 缩进：4个空格（不要用tab，不同编辑器显示不一样）
//   - 行宽：80~120字符，太长了要换行
//   - 空行：逻辑分段用空行隔开
//   - 空格：运算符前后加空格，逗号后面加空格
//   - 括号：函数名和括号之间不加空格，if/for和括号之间加空格
//   - 大括号：要么都换行，要么都不换行，统一就行
// ============================================================

void codeFormatExample()
{
    cout << "\n=== 2. 代码格式 ===" << endl;

    // 好的格式
    int result = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (i % 2 == 0)
        {
            result += i * 2;
        }
        else
        {
            result += i;
        }
    }

    cout << "  好的格式特点：" << endl;
    cout << "    - 缩进统一（4空格）" << endl;
    cout << "    - 大括号对齐" << endl;
    cout << "    - 运算符前后有空格" << endl;
    cout << "    - 逻辑分段有换行" << endl;

    // 坏的格式（反面教材）
    // int r=0;for(int i=0;i<10;i++){if(i%2==0){r+=i*2;}else{r+=i;}}

    cout << "\n  坏的格式：挤成一团，根本没法读" << endl;

    cout << "\n  建议：用格式化工具（clang-format）自动格式化，" << endl;
    cout << "       既省时间又保证一致" << endl;
}

// ============================================================
// 三、注释规范
// ============================================================
// 注释是给人看的，解释"为什么这么写"，而不是"写了什么"。
// 代码本身已经说明了"做了什么"。
//
// 注释原则：
//   1. 好代码 > 坏代码 + 好注释
//   2. 解释为什么（why），而不是是什么（what）
//   3. 复杂算法、业务逻辑、踩过的坑要注释
//   4. 注释要和代码同步更新，过期的注释比没注释还糟
//   5. 别写废话注释（比如 i++; // i加1）
// ============================================================

// ============================================================
// 计算两个日期之间的天数差
// 参数：
//   year1, month1, day1 - 第一个日期
//   year2, month2, day2 - 第二个日期
// 返回值：天数差（正数，取绝对值）
// 注意：使用儒略日算法，支持公元前年份（用负数表示）
// ============================================================
int daysBetweenDates(int year1, int month1, int day1,
                     int year2, int month2, int day2)
{
    // 函数体内部就不详细实现了，这里主要演示注释规范
    // 儒略日转换公式——这玩意儿不注释谁看得懂啊
    // 公式来源：天文学常用的格里高利历转儒略日算法
    auto toJulianDay = [](int y, int m, int d) {
        // 1月2月当作上一年的13月14月处理（这是公式要求的）
        if (m <= 2)
        {
            y--;
            m += 12;
        }
        // 格里高利历修正：每400年减3个闰年
        int e = y / 100;
        int f = 2 - e + e / 4;
        return static_cast<int>(365.25 * (y + 4716))
             + static_cast<int>(30.6001 * (m + 1))
             + d + f - 1524;
    };

    int jd1 = toJulianDay(year1, month1, day1);
    int jd2 = toJulianDay(year2, month2, day2);
    return abs(jd1 - jd2);
}

void commentExample()
{
    cout << "\n=== 3. 注释规范 ===" << endl;

    int days = daysBetweenDates(2024, 1, 1, 2024, 12, 31);
    cout << "  2024年第一天到最后一天相差 " << days << " 天" << endl;

    cout << "\n  注释原则：" << endl;
    cout << "    1. 解释为什么（why），不是是什么（what）" << endl;
    cout << "    2. 复杂算法、踩过的坑一定要注释" << endl;
    cout << "    3. 函数头注释说明用途、参数、返回值、注意事项" << endl;
    cout << "    4. 别写废话：i++;  // i加1（这种就是废话）" << endl;
    cout << "    5. 注释要和代码同步更新，过期注释害人不浅" << endl;
    cout << "    6. 最好的注释：写出让人不用注释也能读懂的代码" << endl;
}

// ============================================================
// 四、接口设计原则
// ============================================================
// 接口是模块之间的契约，设计好了大家都舒服，设计好了天天改。
//
// 设计原则：
//   1. 最小化原则：接口越少越好，只暴露必要的
//   2. 一致性原则：相似的功能用相似的命名和用法
//   3. 不易误用：好的接口让人很难用错
//   4. 错误处理：明确错误怎么返回、怎么处理
//   5. 完整但不过度：够用就行，别过度设计
// ============================================================

// ---- 不好的接口设计 ----
class BadCalculator
{
public:
    // 问题1：参数顺序不明确，调用者容易传反
    double calc(double a, double b, int op)
    {
        // 问题2：用魔法数字表示操作，可读性差
        if (op == 1) return a + b;
        if (op == 2) return a - b;
        // 问题3：除零不处理，直接返回0？还是崩溃？行为不明确
        if (op == 3) return a * b;
        if (op == 4) return a / b;
        return 0;  // 问题4：错误时返回0？调用者怎么知道是错了还是结果就是0？
    }

    // 问题5：暴露内部状态，破坏封装
    double m_result;
    int m_lastOp;
};

// ---- 好的接口设计 ----
enum class Operation
{
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

class Calculator
{
public:
    // 接口清晰：每个操作一个函数，不会传错
    double add(double a, double b) { return a + b; }
    double subtract(double a, double b) { return a - b; }
    double multiply(double a, double b) { return a * b; }

    // 错误处理明确：除零抛异常，调用者必须处理
    double divide(double a, double b)
    {
        if (b == 0)
        {
            throw invalid_argument("除数不能为零");
        }
        return a / b;
    }

    // 也可以用返回值 + 错误码的方式（看场景）
    bool divideSafe(double a, double b, double& result)
    {
        if (b == 0)
            return false;
        result = a / b;
        return true;
    }
    // 内部状态不暴露
};

void interfaceDesign()
{
    cout << "\n=== 4. 接口设计原则 ===" << endl;

    Calculator calc;
    cout << "  好的接口：" << endl;
    cout << "    calc.add(1, 2) = " << calc.add(1, 2) << endl;
    cout << "    calc.multiply(3, 4) = " << calc.multiply(3, 4) << endl;

    try
    {
        calc.divide(10, 0);
    }
    catch (const exception& e)
    {
        cout << "    divide(10, 0) 抛出异常: " << e.what() << endl;
    }

    double result;
    bool ok = calc.divideSafe(10, 0, result);
    cout << "    divideSafe(10, 0) 成功? " << (ok ? "是" : "否") << endl;

    cout << "\n  接口设计原则：" << endl;
    cout << "    1. 最小化：只暴露必要的，藏好实现细节" << endl;
    cout << "    2. 一致性：相似的功能用相似的方式" << endl;
    cout << "    3. 不易误用：让人很难用错（比如强类型、明确的函数名）" << endl;
    cout << "    4. 错误处理明确：要么抛异常，要么返回错误码，别静默失败" << endl;
    cout << "    5. 宁简勿繁：够用就行，别过度设计" << endl;
}

// ============================================================
// 五、异常安全
// ============================================================
// 异常安全是C++的重要课题。
// 异常发生时，程序要保证：
//   - 不泄漏资源
//   - 不破坏数据一致性
//
// 异常安全的三个级别（保证强度递增）：
//
// 1. 基本保证（Basic Guarantee）
//    异常发生后，程序处于有效状态（对象可用、资源不泄漏）
//    但具体状态不确定，可能部分操作完成了
//
// 2. 强保证（Strong Guarantee）
//    异常发生后，程序状态保持不变（就像没调用过一样）
//    典型做法：copy-and-swap
//
// 3. 不抛异常保证（No-Throw Guarantee / Noexcept）
//    保证不会抛出异常
//    比如析构函数、移动操作、swap等
//
// 原则：析构函数绝对不能抛异常！
// ============================================================

// ---- 基本保证的例子：用RAII保证资源不泄漏 ----
class BasicSafeExample
{
public:
    BasicSafeExample() : m_data(new int[100]), m_size(100) {}

    ~BasicSafeExample()
    {
        // 析构函数绝对不能抛异常！
        delete[] m_data;
    }

    // 这个函数提供基本保证：
    // 如果抛异常，对象还是有效的（资源不泄漏）
    void resize(int newSize)
    {
        // 先分配新内存（可能抛bad_alloc）
        int* newData = new int[newSize];

        // 拷贝数据（这里如果抛异常，newData会泄漏！）
        // 所以更好的做法是用智能指针或者copy-and-swap
        for (int i = 0; i < min(m_size, newSize); ++i)
        {
            newData[i] = m_data[i];
        }

        // 释放旧内存（delete不会抛异常）
        delete[] m_data;
        m_data = newData;
        m_size = newSize;
    }

private:
    int* m_data;
    int m_size;
};

// ---- 强保证的例子：copy-and-swap 惯用法 ----
class StrongSafeExample
{
public:
    StrongSafeExample(int size = 10)
        : m_data(size ? new int[size] : nullptr)
        , m_size(size)
    {}

    ~StrongSafeExample() = default;

    // 拷贝构造
    StrongSafeExample(const StrongSafeExample& other)
        : m_data(other.m_size ? new int[other.m_size] : nullptr)
        , m_size(other.m_size)
    {
        for (int i = 0; i < m_size; ++i)
            m_data[i] = other.m_data[i];
    }

    // 拷贝赋值：copy-and-swap，提供强保证
    StrongSafeExample& operator=(const StrongSafeExample& other)
    {
        // 1. 先拷贝一份（可能抛异常，但不影响*this）
        StrongSafeExample temp(other);

        // 2. 再交换（swap不抛异常）
        swap(temp);

        // 3. temp带着旧数据离开作用域，自动释放
        return *this;
    }

    // swap 保证不抛异常
    void swap(StrongSafeExample& other) noexcept
    {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
    }

    // resize 也可以用 copy-and-swap 实现强保证
    void resizeStrong(int newSize)
    {
        // 1. 创建新对象（可能抛异常，但不影响*this）
        StrongSafeExample temp(newSize);
        int copySize = min(m_size, newSize);
        for (int i = 0; i < copySize; ++i)
            temp.m_data[i] = m_data[i];

        // 2. 交换（不抛异常）
        swap(temp);
        // 3. 如果成功，*this就是新状态
        //    如果失败，*this保持原样
    }

    int size() const { return m_size; }

private:
    unique_ptr<int[]> m_data;  // 用智能指针更安全
    int m_size;
};

void exceptionSafety()
{
    cout << "\n=== 5. 异常安全 ===" << endl;

    cout << "  异常安全三个级别：" << endl;
    cout << "    1. 基本保证：异常后程序有效（资源不泄漏、对象可用）" << endl;
    cout << "    2. 强保证：异常后状态不变（像没调用过）" << endl;
    cout << "    3. 不抛异常保证（noexcept）：保证不抛异常" << endl;

    StrongSafeExample a(10);
    StrongSafeExample b(20);
    cout << "\n  copy-and-swap 实现强保证：" << endl;
    cout << "    a.size() = " << a.size() << endl;
    cout << "    b.size() = " << b.size() << endl;

    a = b;  // 如果拷贝过程中抛异常，a保持不变
    cout << "    a = b 后，a.size() = " << a.size() << endl;

    cout << "\n  重要原则：" << endl;
    cout << "    - 析构函数、swap、移动操作 应该是 noexcept 的" << endl;
    cout << "    - RAII 是异常安全的基础" << endl;
    cout << "    - copy-and-swap 是实现强保证的经典手法" << endl;
    cout << "    - 不要在析构函数里抛异常！会直接 terminate" << endl;
}

// ============================================================
// 六、const 正确性
// ============================================================
// const是C++的一大特色，用好const能让代码更安全、更清晰。
//
// const的作用：
//   - 告诉编译器和读者：这个东西不会被修改
//   - 编译器帮你检查，防止意外修改
//   - 可以优化（const对象编译器可以做更多优化）
//
// 原则：能加const就加const！
// ============================================================

class ConstExample
{
public:
    // 构造函数不能const（要初始化成员）
    ConstExample(int value) : m_value(value) {}

    // 不修改成员的函数，要加 const
    int getValue() const { return m_value; }

    // 修改成员的函数，不加const
    void setValue(int value) { m_value = value; }

    // const 成员函数只能调用其他 const 成员函数
    void print() const
    {
        cout << "  值为: " << getValue() << endl;  // 可以，因为getValue是const的
        // setValue(10);  // 错误！const函数里不能调用非const函数
    }

    // 返回const引用，防止外部修改内部数据
    const std::string& getName() const { return m_name; }

    // 重载：const对象调用const版本，非const对象调用非const版本
    int& value() { return m_value; }
    const int& value() const { return m_value; }

private:
    int m_value;
    std::string m_name;

    // mutable 成员：即使在const函数里也能修改
    // 用于不影响逻辑状态的缓存、计数器等
    mutable int m_accessCount = 0;
};

void constCorrectness()
{
    cout << "\n=== 6. const 正确性 ===" << endl;

    ConstExample obj(42);
    cout << "  普通对象：" << endl;
    cout << "    getValue() = " << obj.getValue() << endl;
    obj.setValue(100);
    cout << "    setValue(100) 后 = " << obj.getValue() << endl;
    obj.value() = 200;  // 非const版本，返回引用，可以修改
    cout << "    value() = 200 后 = " << obj.getValue() << endl;

    const ConstExample constObj(50);
    cout << "\n  const对象：" << endl;
    cout << "    getValue() = " << constObj.getValue() << endl;
    // constObj.setValue(100);  // 错误！const对象不能调用非const函数
    cout << "    value() = " << constObj.value() << " （const版本，不能修改）" << endl;

    // 指针的const
    cout << "\n  指针的const（看const在*的哪一边）：" << endl;
    int x = 10, y = 20;

    const int* p1 = &x;  // 指向const的指针：不能通过指针修改值
    // *p1 = 100;  // 错误
    p1 = &y;  // 可以，指针本身可以改
    cout << "    const int* p: 指向const，指针可变，值不可变" << endl;

    int* const p2 = &x;  // const指针：指针本身不能改
    *p2 = 100;  // 可以，值可以改
    // p2 = &y;  // 错误
    cout << "    int* const p: const指针，指针不可变，值可变" << endl;

    const int* const p3 = &x;  // 都不能改
    cout << "    const int* const p: 都不能改" << endl;

    cout << "\n  记忆技巧：const 在 * 左边 → 指向的内容是const" << endl;
    cout << "           const 在 * 右边 → 指针本身是const" << endl;

    cout << "\n  const 原则：能加const就加const！" << endl;
    cout << "    - 不修改的成员函数 → 加const" << endl;
    cout << "    - 只读的函数参数 → 加const引用" << endl;
    cout << "    - 不修改的局部变量 → 加const" << endl;
    cout << "    - 返回内部数据的引用 → 返回const引用" << endl;
}

// ============================================================
// 七、常见陷阱与避坑指南
// ============================================================

// 7.1 隐式转换陷阱
class Integer
{
public:
    // 单参数构造函数默认支持隐式转换
    // 用 explicit 禁止隐式转换，避免意外
    explicit Integer(int value) : m_value(value) {}

    int value() const { return m_value; }
private:
    int m_value;
};

void printInteger(Integer /*i*/)
{
    // cout << i.value() << endl;
}

void trap_implicitConversion()
{
    cout << "\n=== 7.1 隐式转换陷阱 ===" << endl;

    // 如果构造函数不加explicit，下面这样也能编译通过
    // printInteger(42);  // int 隐式转成 Integer，可能不是你想要的

    // 加了explicit之后，必须显式构造
    printInteger(Integer(42));
    cout << "  加了 explicit，必须显式构造，避免意外转换" << endl;

    cout << "\n  经验：单参数构造函数默认加 explicit，除非你确实想要隐式转换" << endl;
}

// 7.2 自赋值陷阱
class BadString
{
public:
    BadString(const char* str = "")
    {
        m_data = new char[strlen(str) + 1];
        strcpy(m_data, str);
    }

    ~BadString() { delete[] m_data; }

    // 错误的赋值运算符：自赋值会崩溃！
    BadString& operator=(const BadString& other)
    {
        // 如果 other 就是 *this 呢？
        delete[] m_data;           // 先把自己的删了
        m_data = new char[strlen(other.m_data) + 1];  // other.m_data 已经被删了！
        strcpy(m_data, other.m_data);
        return *this;
    }

private:
    char* m_data;
};

class GoodString
{
public:
    GoodString(const char* str = "")
    {
        m_data = new char[strlen(str) + 1];
        strcpy(m_data, str);
    }

    ~GoodString() { delete[] m_data; }

    // 正确的赋值运算符：先检查自赋值
    GoodString& operator=(const GoodString& other)
    {
        if (this != &other)  // 自赋值检查
        {
            // 或者更好的方式：copy-and-swap（前面讲过）
            char* newData = new char[strlen(other.m_data) + 1];
            strcpy(newData, other.m_data);
            delete[] m_data;
            m_data = newData;
        }
        return *this;
    }

    const char* c_str() const { return m_data; }

private:
    char* m_data;
};

void trap_selfAssignment()
{
    cout << "\n=== 7.2 自赋值陷阱 ===" << endl;

    GoodString s("hello");
    s = s;  // 自赋值，有检查就没问题
    cout << "  s = s 自赋值，有检查就不会崩" << endl;
    cout << "  更推荐用 copy-and-swap，不用额外检查也安全" << endl;
}

// 7.3 悬空引用/指针陷阱
int& danglingRefBad()
{
    int x = 10;
    return x;  // 错误！返回局部变量的引用，函数结束x就没了
}

void trap_danglingRef()
{
    cout << "\n=== 7.3 悬空引用/指针 ===" << endl;
    cout << "  错误示例：返回局部变量的引用或指针" << endl;
    cout << "    int& bad() { int x = 10; return x; } // 大错特错！" << endl;
    cout << "    int* bad() { int x = 10; return &x; } // 一样错！" << endl;
    cout << "  后果：返回的是悬空引用/指针，访问就是未定义行为" << endl;
    cout << "  正确：返回值拷贝（小对象），或者返回动态分配的对象的指针" << endl;

    // 另一个常见场景：range-based for 里修改容器
    vector<int> vec = {1, 2, 3};
    cout << "\n  另一个坑：遍历vector时push_back导致迭代器失效" << endl;
    cout << "    for (int x : vec) { vec.push_back(x * 2); } // 危险！" << endl;
    cout << "  push_back可能触发扩容，迭代器全部失效" << endl;
}

// 7.4 操作符优先级陷阱
void trap_operatorPrecedence()
{
    cout << "\n=== 7.4 操作符优先级陷阱 ===" << endl;

    int a = 5, b = 3;
    int result = a & b == 3;  // 你以为是 (a&b) == 3？
    // 实际上 == 优先级比 & 高，所以是 a & (b == 3) → a & 1 → 1
    cout << "  a & b == 3 的结果: " << result << "（不是你想的那样！）" << endl;
    cout << "  实际运算顺序：a & (b == 3)，因为 == 比 & 优先级高" << endl;

    cout << "\n  经验：拿不准优先级就加括号，代码是写给人看的" << endl;
    cout << "       (a & b) == 3  这样写谁都不会看错" << endl;
}

// 7.5 浮点比较陷阱
void trap_floatCompare()
{
    cout << "\n=== 7.5 浮点数比较陷阱 ===" << endl;

    double a = 0.1 + 0.2;
    double b = 0.3;
    cout << "  0.1 + 0.2 = " << a << endl;
    cout << "  0.1 + 0.2 == 0.3 ? " << (a == b ? "true" : "false") << endl;
    cout << "  （浮点数有精度误差，不能直接用==比较）" << endl;

    // 正确做法：用一个很小的epsilon做范围比较
    const double EPS = 1e-9;
    bool equal = abs(a - b) < EPS;
    cout << "  正确比较：abs(a - b) < 1e-9 ? " << (equal ? "true" : "false") << endl;
}

// ============================================================
// 八、更多最佳实践
// ============================================================

void moreBestPractices()
{
    cout << "\n============================================================" << endl;
    cout << "  八、更多最佳实践" << endl;
    cout << "============================================================" << endl;
    cout << endl;

    cout << "  1. 用 C++ 的方式写 C++，不是 C with classes" << endl;
    cout << "     - 用 vector/string 代替 裸数组/char*" << endl;
    cout << "     - 用智能指针代替 new/delete" << endl;
    cout << "     - 用 STL 算法代替手写循环" << endl;
    cout << "     - 用引用代替指针（能不用指针就不用）" << endl;
    cout << endl;

    cout << "  2. 小对象直接传值，大对象传const引用" << endl;
    cout << "     - int, double, char 这种 → 直接传值" << endl;
    cout << "     - string, vector, 自定义类 → 传const引用" << endl;
    cout << "     - 要修改的 → 传引用（慎用，优先用返回值）" << endl;
    cout << endl;

    cout << "  3. 尽量用初始化列表，少用赋值" << endl;
    cout << "     - 构造函数里用初始化列表，效率更高" << endl;
    cout << "     - 成员是const或引用的，必须用初始化列表" << endl;
    cout << endl;

    cout << "  4. 避免全局变量" << endl;
    cout << "     - 全局变量破坏封装，增加耦合" << endl;
    cout << "     - 多线程下全局变量是噩梦" << endl;
    cout << "     - 实在要用，考虑单例模式或者命名空间里的静态变量" << endl;
    cout << endl;

    cout << "  5. 头文件里少包含，多用前向声明" << endl;
    cout << "     - 只在头文件里放必须的include" << endl;
    cout << "     - 只声明指针/引用的类，用前向声明就行" << endl;
    cout << "     - 实现文件（.cpp）里再include需要的头文件" << endl;
    cout << "     - 好处：编译快，减少依赖" << endl;
    cout << endl;

    cout << "  6. 一个函数只做一件事" << endl;
    cout << "     - 函数太长就拆，嵌套太深就抽" << endl;
    cout << "     - 好的函数应该一屏能看完" << endl;
    cout << "     - 函数名要准确描述它做什么" << endl;
    cout << endl;

    cout << "  7. 别过早优化，但也要心里有数" << endl;
    cout << "     - 先写对，再写好" << endl;
    cout << "     - 性能瓶颈靠测，不靠猜" << endl;
    cout << "     - 但数据结构和算法的选择要一开始就想清楚" << endl;
}

// ============================================================
// 主函数
// ============================================================
int main()
{
    cout << "============================================================" << endl;
    cout << "  编程规范与最佳实践 - 中级进阶Demo" << endl;
    cout << "============================================================" << endl;

    // 一、命名规范
    goodNamingExample();

    // 二、代码格式
    codeFormatExample();

    // 三、注释规范
    commentExample();

    // 四、接口设计
    interfaceDesign();

    // 五、异常安全
    exceptionSafety();

    // 六、const正确性
    constCorrectness();

    // 七、常见陷阱
    trap_implicitConversion();
    trap_selfAssignment();
    trap_danglingRef();
    trap_operatorPrecedence();
    trap_floatCompare();

    // 八、更多最佳实践
    moreBestPractices();

    // ============================================================
    // 老师傅的话
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  老师傅的话" << endl;
    cout << "============================================================" << endl;
    cout << "  徒弟啊，今天讲的这些，看起来都是细枝末节，" << endl;
    cout << "  但真正拉开差距的，往往就是这些细节。" << endl;
    cout << endl;
    cout << "  1. 代码是写给人看的，顺便给机器执行。" << endl;
    cout << "     你写的代码，三个月后你自己还能看懂吗？" << endl;
    cout << "     你的同事接手你的代码，会不会骂娘？" << endl;
    cout << "     写代码的时候多想想读代码的人。" << endl;
    cout << endl;
    cout << "  2. 命名是程序员的基本功，也是最难的事。" << endl;
    cout << "     好名字让人一看就懂，坏名字让人猜半天。" << endl;
    cout << "     想不出好名字的时候，说明你对问题理解还不够深。" << endl;
    cout << "     多花几秒钟想名字，以后能省几小时debug时间。" << endl;
    cout << endl;
    cout << "  3. const 正确性非常重要。" << endl;
    cout << "     能加const的地方都加上，" << endl;
    cout << "     编译器帮你检查，读者一看就懂。" << endl;
    cout << "     这是成本最低、收益最高的好习惯之一。" << endl;
    cout << endl;
    cout << "  4. 异常安全不是可有可无的。" << endl;
    cout << "     你写的每个函数，都要想想：" << endl;
    cout << "     如果中间抛异常了，资源会泄漏吗？数据会乱吗？" << endl;
    cout << "     RAII 是基础，copy-and-swap 是利器。" << endl;
    cout << endl;
    cout << "  5. 接口设计是艺术。" << endl;
    cout << "     好的接口让人很难用错，坏的接口让人防不胜防。" << endl;
    cout << "     设计接口的时候，多站在调用者的角度想想。" << endl;
    cout << "     接口一旦发布，改起来就难了，要慎重。" << endl;
    cout << endl;
    cout << "  6. 保持谦逊，持续学习。" << endl;
    cout << "     C++水很深，没有人敢说自己全懂。" << endl;
    cout << "     多看高手的代码，多反思自己的代码。" << endl;
    cout << "     代码写出来不是结束，是开始——" << endl;
    cout << "     以后还要维护、还要重构、还要读。" << endl;
    cout << "============================================================" << endl;

    // ============================================================
    // 小练习
    // ============================================================
    cout << "\n============================================================" << endl;
    cout << "  小练习（回去自己做）" << endl;
    cout << "============================================================" << endl;
    cout << "  1. 找一段你以前写的代码，按照今天讲的规范重构一遍" << endl;
    cout << "     重点看：命名、注释、const、异常安全" << endl;
    cout << "  2. 设计一个字符串类（MyString），要求：" << endl;
    cout << "     - 支持拷贝构造、拷贝赋值（用copy-and-swap实现强保证）" << endl;
    cout << "     - 支持移动构造、移动赋值（noexcept）" << endl;
    cout << "     - const 成员函数都加好const" << endl;
    cout << "     - 接口设计合理，不易误用" << endl;
    cout << "  3. 思考：为什么析构函数不能抛异常？" << endl;
    cout << "     如果析构函数里可能出错，该怎么处理？" << endl;
    cout << "  4. 阅读一份开源C++项目的代码（比如fmt、spdlog），" << endl;
    cout << "     学习人家的命名、格式、接口设计" << endl;
    cout << "  5. 配置一个 clang-format，定义你自己的代码风格" << endl;
    cout << "============================================================" << endl;

    return 0;
}
