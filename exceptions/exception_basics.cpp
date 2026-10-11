// ============================================================
// exception_basics.cpp —— 异常基础与异常安全
// ============================================================
//
// 编译命令：
//   g++ -std=c++17 exception_basics.cpp -o demo
//
// ============================================================

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>   // 智能指针
#include <mutex>    // lock_guard
#include <fstream>

// ============================================================
// 一、throw / try / catch 基本用法
// ============================================================
//
// 异常处理三件套：
//   throw  —— 抛出异常（出事了，喊一声）
//   try    —— 尝试执行可能出问题的代码
//   catch  —— 捕获并处理异常
//
// 流程：try 块里的代码抛出异常 → 跳到匹配的 catch 块 → 处理完继续走
//       如果没匹配的 catch，程序就 terminate（崩了）

// 一个可能抛出异常的函数
double safeDivide(double a, double b) {
    if (b == 0) {
        // throw 抛出一个异常对象，可以是任何类型
        // 通常抛标准异常类的对象
        throw std::runtime_error("除数不能为零");
    }
    return a / b;
}

void demo_basic_try_catch() {
    std::cout << "=== 一、try/catch 基本用法 ===" << std::endl;

    try {
        double result = safeDivide(10, 2);
        std::cout << "10 / 2 = " << result << std::endl;

        result = safeDivide(5, 0);  // 这里会抛异常
        std::cout << "这行不会执行" << std::endl;

    } catch (const std::runtime_error& e) {
        // 捕获到异常，e.what() 返回错误信息
        std::cout << "捕获到异常：" << e.what() << std::endl;
    }

    std::cout << "程序继续运行..." << std::endl;
    std::cout << std::endl;
}

// 多个 catch 块：按顺序匹配，第一个匹配的执行
void demo_multiple_catch() {
    std::cout << "=== 多个 catch 块 ===" << std::endl;

    try {
        // 试着改改这里抛出的类型，看看进哪个 catch
        throw std::logic_error("逻辑错误");
        // throw std::runtime_error("运行时错误");
        // throw std::string("字符串异常");

    } catch (const std::logic_error& e) {
        std::cout << "logic_error: " << e.what() << std::endl;

    } catch (const std::runtime_error& e) {
        std::cout << "runtime_error: " << e.what() << std::endl;

    } catch (const std::exception& e) {
        // 所有标准异常都继承自 std::exception
        // 放在最后当"兜底"
        std::cout << "标准异常: " << e.what() << std::endl;

    } catch (...) {
        // 三个点表示捕获任何类型的异常
        // 最兜底的方案，但拿不到异常信息
        std::cout << "捕获到未知类型的异常" << std::endl;
    }

    std::cout << std::endl;
}

// 异常的传播：如果当前函数没 catch，会往上抛给调用者
void level3() {
    throw std::runtime_error("level3 里出的错");
}

void level2() {
    level3();  // 没 catch，继续往上抛
}

void level1() {
    try {
        level2();
    } catch (const std::exception& e) {
        std::cout << "level1 捕获到：" << e.what() << std::endl;
    }
}

void demo_exception_propagation() {
    std::cout << "=== 异常传播（栈展开） ===" << std::endl;
    level1();
    std::cout << std::endl;
}

// ============================================================
// 二、标准异常类层次
// ============================================================
//
// std::exception 是所有标准异常的基类，有个 what() 虚函数。
//
// 主要派生类分两大派系：
//
//   std::logic_error（逻辑错误，理论上可以避免）
//     ├── std::invalid_argument   无效参数
//     ├── std::length_error       长度超出限制
//     ├── std::out_of_range       下标越界
//     └── std::domain_error       定义域错误（比如 sqrt(-1)）
//
//   std::runtime_error（运行时错误，程序运行时才知道）
//     ├── std::range_error        结果超出范围
//     ├── std::overflow_error     上溢
//     ├── std::underflow_error    下溢
//     └── std::system_error       系统调用错误（C++11）
//
// 怎么选？
//   - 是调用者传错了参数导致的 → logic_error 派系
//   - 是运行时环境/状态导致的   → runtime_error 派系
//   - 拿不准就 runtime_error，大差不差

void demo_standard_exceptions() {
    std::cout << "=== 二、标准异常类层次 ===" << std::endl;

    // 示例：vector 的 at() 会抛 out_of_range
    std::vector<int> vec = {1, 2, 3};
    try {
        int val = vec.at(10);  // at() 会做边界检查，越界抛异常
        std::cout << "值：" << val << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "越界异常：" << e.what() << std::endl;
    }

    // 注意：operator[] 不抛异常，越界是未定义行为！
    // vec[10];  // 这行不要随便试，可能崩可能不崩

    std::cout << std::endl;
}

// ============================================================
// 三、自定义异常类
// ============================================================
// 标准异常不够用？自己写一个！
// 通常继承 std::runtime_error 或 std::logic_error 就行。

// 自定义异常：文件读取失败
class FileReadError : public std::runtime_error {
public:
    FileReadError(const std::string& filename, const std::string& reason)
        : std::runtime_error("读取文件失败: " + filename + " - " + reason),
          filename_(filename),
          reason_(reason) {}

    // 可以加额外的成员函数，携带更多信息
    const std::string& filename() const { return filename_; }
    const std::string& reason() const { return reason_; }

private:
    std::string filename_;
    std::string reason_;
};

// 自定义异常：网络错误（带错误码）
class NetworkError : public std::runtime_error {
public:
    NetworkError(int code, const std::string& msg)
        : std::runtime_error("网络错误 [" + std::to_string(code) + "]: " + msg),
          error_code_(code) {}

    int error_code() const { return error_code_; }

private:
    int error_code_;
};

void readConfigFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw FileReadError(path, "文件不存在或无法打开");
    }
    // ... 读取内容
}

void demo_custom_exceptions() {
    std::cout << "=== 三、自定义异常类 ===" << std::endl;

    try {
        readConfigFile("nonexistent.cfg");
    } catch (const FileReadError& e) {
        std::cout << e.what() << std::endl;
        std::cout << "文件名：" << e.filename() << std::endl;
        std::cout << "原因：" << e.reason() << std::endl;
    }

    std::cout << std::endl;
}

// ============================================================
// 四、RAII 与异常安全
// ============================================================
//
// RAII = Resource Acquisition Is Initialization
//        资源获取即初始化
//
// 核心思想：
//   用对象管理资源，构造时获取，析构时释放。
//   不管函数怎么返回（正常 return 还是异常抛出），
//   局部对象的析构函数都会被调用——资源总能释放。
//
// 这就是 C++ 处理异常安全的核心手段！

// 反面教材：手动管理资源，遇到异常就泄漏
void bad_example_resource_leak() {
    std::cout << "=== 四、RAII 与异常安全 ===" << std::endl;
    std::cout << "--- 反面教材：手动 new，异常会泄漏 ---" << std::endl;

    int* p = new int(42);  // 手动分配

    // 如果中间这里抛异常了怎么办？
    // throw std::runtime_error("意外错误");
    // 下面的 delete 就执行不到了！内存泄漏！

    std::cout << "*p = " << *p << std::endl;
    delete p;  // 只有正常走到这里才释放
}

// 正面教材：用智能指针，怎么抛都安全
void good_example_raii() {
    std::cout << "--- 正面教材：智能指针（RAII） ---" << std::endl;

    std::unique_ptr<int> p = std::make_unique<int>(42);

    // 就算这里抛异常，p 的析构函数也会被调用
    // 因为局部对象在栈展开时一定会析构
    // throw std::runtime_error("意外错误");  // 取消注释试试

    std::cout << "*p = " << *p << std::endl;
    // 不用手动 delete，离开作用域自动释放
}

// 另一个例子：lock_guard 管理互斥锁
// 不管函数怎么退出，锁都会释放
std::mutex mtx;

void safeFunction() {
    std::lock_guard<std::mutex> lock(mtx);  // 构造时加锁

    // 就算这里抛异常，lock 的析构函数也会调用 unlock()
    // 不会死锁！

    // ... 操作共享资源
}

// 手写一个 RAII 类，理解原理
class RAIIFile {
public:
    RAIIFile(const std::string& path) {
        fp = std::fopen(path.c_str(), "r");
        std::cout << "文件打开了" << std::endl;
    }

    ~RAIIFile() {
        if (fp) {
            std::fclose(fp);
            std::cout << "文件关闭了（析构函数自动调用）" << std::endl;
        }
    }

    // 禁止拷贝（防止重复释放）
    RAIIFile(const RAIIFile&) = delete;
    RAIIFile& operator=(const RAIIFile&) = delete;

private:
    FILE* fp = nullptr;
};

void demo_raii_file() {
    std::cout << "--- 自定义 RAII 类示例 ---" << std::endl;
    try {
        RAIIFile file("test.txt");  // 可能打开失败，但析构照样调用
        // ... 做一些操作
        throw std::runtime_error("中间出了点问题");
        // 就算抛异常，file 的析构函数也会跑
    } catch (const std::exception& e) {
        std::cout << "捕获异常：" << e.what() << std::endl;
    }
    std::cout << std::endl;
}

// ============================================================
// 五、异常安全的三个级别
// ============================================================
//
// 写函数的时候，要考虑"如果抛异常了，对象状态还对吗"。
// 异常安全有三个级别，从低到高：
//
//  1. 基本保证（Basic Guarantee）
//     抛异常后，对象处于合法但不确定的状态。
//     至少不会泄漏资源，不会崩溃。
//     比如：vector push_back 失败了，vector 还是能用的，
//     只是不确定元素加没加上。
//
//  2. 强保证（Strong Guarantee）
//     抛异常后，对象状态保持不变（就像没调用过这个函数一样）。
//     典型做法：copy-and-swap（先复制修改，成功了再交换）。
//     要么全成功，要么全失败，不留中间状态。
//
//  3. 不抛出保证（No-throw Guarantee / No-fail Guarantee）
//     保证绝对不抛异常，一定成功。
//     比如：基本类型操作、析构函数、swap 函数。
//     用 noexcept 标记。

// 演示：强保证的实现方式——copy-and-swap
class StringList {
public:
    StringList() = default;

    // 深拷贝构造
    StringList(const StringList& other) : data_(other.data_) {}

    // 添加元素——提供强保证
    // 要么成功加上，要么原样不变
    void addStrong(const std::string& s) {
        // 1. 先做一份拷贝
        std::vector<std::string> temp = data_;

        // 2. 在拷贝上修改（这一步可能抛异常，比如内存分配失败）
        temp.push_back(s);

        // 3. 修改成功了，再交换（swap 不抛异常）
        data_.swap(temp);

        // 这样就算 push_back 抛异常，data_ 还是原来的样子
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<std::string> data_;
};

void demo_exception_safety_levels() {
    std::cout << "=== 五、异常安全的三个级别 ===" << std::endl;
    std::cout << "级别 1：基本保证——不泄漏、不崩溃，状态合法但不确定" << std::endl;
    std::cout << "级别 2：强保证——要么全成功，要么全失败，状态不变" << std::endl;
    std::cout << "级别 3：不抛出保证——绝对不抛异常，一定成功" << std::endl;

    StringList list;
    list.addStrong("hello");
    list.addStrong("world");
    std::cout << "StringList size: " << list.size() << std::endl;
    std::cout << std::endl;
}

// ============================================================
// 六、noexcept 关键字
// ============================================================
//
// noexcept 告诉编译器：这个函数保证不抛异常。
//
// 作用：
//   1. 优化：编译器知道不会抛异常，可以生成更高效的代码
//   2. 语义：让调用者放心，不用写 try/catch
//   3. 移动语义：容器在扩容时，只有元素的移动构造是 noexcept 的，
//      才会用移动（否则用拷贝，保证强异常安全）
//
// 注意：noexcept 不是强制的！
//       如果你标记了 noexcept 但函数里还是抛了异常，
//       程序会直接调用 std::terminate() 终止。
//       所以 noexcept 是承诺，要说到做到。

// 简单的 noexcept 函数
int add(int a, int b) noexcept {
    return a + b;  // 基本类型运算不会抛异常
}

// 类的成员函数也可以标 noexcept
class NoThrowExample {
public:
    NoThrowExample() noexcept = default;
    ~NoThrowExample() noexcept = default;  // 析构函数默认就是 noexcept 的

    // 移动构造——强烈建议标 noexcept！
    NoThrowExample(NoThrowExample&& other) noexcept {
        // 移动资源...
    }

    // 交换函数——通常也不抛异常
    void swap(NoThrowExample& other) noexcept {
        // 交换成员...
    }
};

// noexcept 也可以带条件（noexcept(表达式)）
// 表达式为 true 则不抛，false 则可能抛
template <typename T>
void process(T& obj) noexcept(noexcept(obj.doSomething())) {
    // 如果 obj.doSomething() 不抛异常，这个函数也不抛
    obj.doSomething();
}

void demo_noexcept() {
    std::cout << "=== 六、noexcept 关键字 ===" << std::endl;

    std::cout << "add 是 noexcept 的吗？ "
              << (noexcept(add(1, 2)) ? "是" : "否") << std::endl;

    std::cout << "safeDivide 是 noexcept 的吗？ "
              << (noexcept(safeDivide(1, 2)) ? "是" : "否") << std::endl;

    // 注意：noexcept 是运算符，可以在编译期查询函数是否承诺不抛异常
    // 语法：noexcept(表达式)  如果表达式不会抛异常则返回 true

    std::cout << std::endl;
    std::cout << "什么时候用 noexcept？" << std::endl;
    std::cout << "  - 析构函数（默认就是，不用手动写）" << std::endl;
    std::cout << "  - 移动构造 / 移动赋值（强烈建议！影响性能）" << std::endl;
    std::cout << "  - swap 函数" << std::endl;
    std::cout << "  - 简单的 getter / 基本运算" << std::endl;
    std::cout << "  - 任何你确定不会抛异常的函数" << std::endl;
    std::cout << std::endl;
}

// ============================================================
// 七、异常 vs 错误码
// ============================================================
//
// 两种错误处理方式各有优劣，不是谁取代谁。
//
// 【异常的优点】
//   1. 无法忽略：错误码可以不检查，异常不行（不 catch 就崩）
//   2. 代码干净：正常逻辑和错误处理分开，不用每层都判断返回值
//   3. 自带信息：异常对象可以带丰富的错误信息
//   4. 构造函数只能用异常：构造函数没有返回值，出错只能抛异常
//
// 【异常的缺点】
//   1. 性能开销：有一定的运行时开销（虽然现代编译器优化得不错了）
//   2. 控制流不直观：throw 跳到哪了？得跟着 catch 找
//   3. 异常安全难写：写出正确的异常安全代码需要经验
//   4. 二进制兼容：不同编译器的异常实现可能不一样
//
// 【怎么选？】
//   - 预期内的、频繁发生的错误 → 错误码（比如用户输入验证）
//   - 意料之外的、罕见的错误 → 异常（比如内存不足、文件损坏）
//   - 构造函数出错 → 异常（没别的选择）
//   - 析构函数 → 绝对不能抛异常！
//   - 回调函数 / 信号处理 → 通常用错误码（异常不好跨边界）
//
// C++ 社区的共识是：异常是主流，但要用得谨慎。
// 不是所有错误都要抛异常，也不是所有地方都适合用异常。

// 错误码风格（C 风格）
int divide_error_code(int a, int b, int* result) {
    if (b == 0) {
        return -1;  // 返回错误码
    }
    *result = a / b;
    return 0;  // 成功
}

// 异常风格
int divide_exception(int a, int b) {
    if (b == 0) {
        throw std::invalid_argument("除数为零");
    }
    return a / b;
}

void demo_error_vs_exception() {
    std::cout << "=== 七、异常 vs 错误码 ===" << std::endl;

    // 错误码风格：每层都要检查
    int result;
    if (divide_error_code(10, 0, &result) != 0) {
        std::cout << "错误码方式：出错了" << std::endl;
    }

    // 异常风格：正常逻辑和错误处理分离
    try {
        int r = divide_exception(10, 2);
        std::cout << "异常方式：结果 = " << r << std::endl;
    } catch (const std::exception& e) {
        std::cout << "异常方式：出错了 - " << e.what() << std::endl;
    }

    std::cout << std::endl;
    std::cout << "老师傅建议：" << std::endl;
    std::cout << "  能用异常的地方用异常，但要注意异常安全。" << std::endl;
    std::cout << "  性能敏感或跨语言边界的地方用错误码。" << std::endl;
    std::cout << "  关键是：项目里要统一，不要一会儿异常一会儿错误码。" << std::endl;
    std::cout << std::endl;
}

// ============================================================
// 八、经典反例
// ============================================================
//
// 反例 1：构造函数里抛异常
//   这其实是对的——构造函数出错就该抛异常。
//   但要注意：如果构造函数抛异常，析构函数不会被调用！
//   因为对象还没构造完成。
//   所以构造函数里分配了资源又抛异常，要自己在构造函数里清理，
//   或者用 RAII 成员（成员的析构函数会被调用）。

class BadConstructor {
public:
    BadConstructor() {
        p1 = new int(1);
        // 如果这里抛异常，p1 泄漏了！
        // 因为析构函数不会被调用
        throw std::runtime_error("构造失败");
        p2 = new int(2);  // 这行执行不到
    }

    ~BadConstructor() {
        delete p1;
        delete p2;
    }

private:
    int* p1 = nullptr;
    int* p2 = nullptr;
};

// 正确做法：用智能指针当成员
class GoodConstructor {
public:
    GoodConstructor() {
        p1 = std::make_unique<int>(1);
        // 就算这里抛异常，p1 的析构函数会被调用
        // 因为成员对象的析构是自动的
        throw std::runtime_error("构造失败");
    }

    // 不用手动写析构了，智能指针自动释放

private:
    std::unique_ptr<int> p1;
    std::unique_ptr<int> p2;
};

// 反例 2：析构函数里抛异常
//   这是大忌！绝对不要在析构函数里抛异常！
//   原因：如果栈展开时（已经在处理异常了）又抛一个异常，
//   C++ 会直接调用 std::terminate() 终止程序。
//   两个异常同时存在，C++ 不知道该处理哪个。

class BadDestructor {
public:
    ~BadDestructor() {
        // 千万别这么写！
        // throw std::runtime_error("析构函数抛异常");
        // 如果在栈展开时触发，直接 terminate
    }
};

// 正确做法：析构函数里吞掉异常，或者记录日志
class GoodDestructor {
public:
    ~GoodDestructor() {
        try {
            // 可能抛异常的清理工作
            // closeFile();
        } catch (const std::exception& e) {
            // 记录日志，但别往外抛！
            std::cerr << "清理失败：" << e.what() << std::endl;
        }
    }
};

void demo_anti_patterns() {
    std::cout << "=== 八、经典反例 ===" << std::endl;
    std::cout << "反例 1：构造函数里抛异常" << std::endl;
    std::cout << "  → 本身没问题，但要注意资源清理，用 RAII 成员" << std::endl;
    std::cout << std::endl;
    std::cout << "反例 2：析构函数里抛异常" << std::endl;
    std::cout << "  → 绝对禁止！栈展开时双重异常会直接 terminate" << std::endl;
    std::cout << std::endl;

    // 试试构造函数抛异常的情况
    std::cout << "测试：构造函数抛异常，智能指针成员会自动释放吗？" << std::endl;
    try {
        GoodConstructor obj;  // 构造会抛异常
    } catch (const std::exception& e) {
        std::cout << "捕获到构造异常：" << e.what() << std::endl;
        std::cout << "（智能指针成员已经自动释放了，放心）" << std::endl;
    }
    std::cout << std::endl;
}

// ============================================================
// main 函数
// ============================================================
int main() {
    demo_basic_try_catch();
    demo_multiple_catch();
    demo_exception_propagation();
    demo_standard_exceptions();
    demo_custom_exceptions();
    bad_example_resource_leak();
    good_example_raii();
    demo_raii_file();
    demo_exception_safety_levels();
    demo_noexcept();
    demo_error_vs_exception();
    demo_anti_patterns();

    std::cout << "程序正常结束" << std::endl;
    return 0;
}

// ============================================================
// 老师傅的话
// ============================================================
//
// 小年轻，看到这儿说明你把异常这块儿啃下来了。
// 老师傅我跟你说几句掏心窝子的。
//
// 【异常不是洪水猛兽】
// 很多人被"异常性能开销"吓住了，打死不用异常。
// 说实话，现代 CPU 和编译器下，只要异常不频繁抛出，
// 那点开销你根本感觉不到。
// 真正影响性能的是频繁抛异常（比如循环里抛），
// 但异常本来就不该用来处理"正常流程"的事。
// 异常是给"异常情况"用的——这句话很绕，但就是这个意思。
//
// 【异常安全是 C++ 程序员的必修课】
// Java、Python 里你不用太操心异常安全，
// 因为有垃圾回收，资源不用手动管。
// 但 C++ 不一样，内存、文件、锁……都得自己管。
// 一不小心抛个异常，资源就泄漏了。
//
// 所以学 C++ 一定要学 RAII，一定要理解异常安全。
// 这是 C++ 和其他语言最大的区别之一，
// 也是 C++ 程序员水平的分水岭。
//
// 【记住 RAII 这四个字】
// RAII 不是什么高大上的设计模式，就是个朴素的思想：
// "资源交给对象管，对象死了资源自动放。"
//
// 智能指针、lock_guard、fstream、string、vector……
// 标准库里到处都是 RAII 的例子。
// 你写代码的时候也要养成习惯：
// 凡是要管理资源的地方，就想能不能用 RAII 包一层。
// 这样不管抛不抛异常，你的代码都是安全的。
//
// 【关于 noexcept】
// 新手容易两个极端：要么全标 noexcept，要么一个都不标。
// 正确的做法是：确定不会抛的才标，不确定的就别瞎承诺。
// 尤其注意移动构造和移动赋值——标 noexcept 对性能影响很大。
// 容器（vector 等）在扩容的时候，
// 只有元素的移动构造是 noexcept 的，才会用移动，
// 否则只能用拷贝（为了保证强异常安全）。
// 这性能差距可就大了。
//
// 【最后一句话】
// 异常这东西，理解起来不难，用好不容易。
// 关键是要有"异常安全意识"——
// 写每一行代码的时候都想想：
// "如果这行抛异常了，前面申请的资源能释放吗？"
// 有这个意识，你就超过一半的 C++ 程序员了。
//
// ============================================================
// 小练习
// ============================================================
//
// 1. 基础练习：
//    写一个函数 int stringToInt(const string& s)，
//    如果字符串不是合法的整数，就抛 std::invalid_argument 异常。
//    然后在 main 里 try/catch 调用它，测试各种情况：
//    正常数字、负数、带字母的、空字符串、超大数字等。
//
// 2. 自定义异常练习：
//    设计一个自定义异常层次：
//      BaseException（继承自 std::exception）
//        ├── DatabaseException
//        │     ├── ConnectionError
//        │     └── QueryError
//        └── NetworkException
//              ├── TimeoutError
//              └── ProtocolError
//    每个异常都要有自己的 what() 消息和额外信息（比如错误码）。
//    然后写几个函数模拟抛这些异常，测试 catch 是否按预期匹配。
//
// 3. RAII 练习：
//    手写一个 RAII 类来管理 FILE*（C 风格文件指针）。
//    要求：
//      - 构造函数打开文件
//      - 析构函数关闭文件
//      - 禁止拷贝
//      - 提供 get() 方法返回原始指针
//      - 提供 release() 方法释放所有权（返回指针并不再管理）
//    然后写个测试，验证抛异常时文件确实被关闭了。
//
// 4. 异常安全练习：
//    写一个简单的动态数组类（类似简化版 vector），
//    包含 push_back 方法。
//    要求 push_back 提供强异常保证：
//      - 如果内存分配失败，数组保持原样
//    提示：用 copy-and-swap 思路，或者先分配新空间再拷贝元素。
//
// 5. 思考题：
//    为什么 C++ 标准库的 vector 的 push_back 提供的是"强保证"，
//    而不是"不抛出保证"？
//    （提示：想想元素的拷贝/移动构造会不会抛异常）
//
// ============================================================
