/*
 * ============================================================
 *   调试入门：printf 调试大法
 *   老师傅说：printf 调试是最笨但也最靠谱的方法
 * ============================================================
 *
 *  学完这个文件，你要学会：
 *    1. 什么时候该用 printf 调试
 *    2. 怎么打日志才高效
 *    3. 调试输出的几个层次
 *    4. 二分法定位 bug
 */

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>

// ============================================================
//  第一招：条件编译 —— 调试版本有输出，发布版本干干净净
// ============================================================

// 技巧：用 NDEBUG 宏控制调试输出
// 编译时加 -DNDEBUG 就能关掉所有调试输出
// （assert 宏也是用的这个机制哦！）

#ifdef NDEBUG
    #define DEBUG_LOG(msg)   ((void)0)   // 发布版：什么都不做
    #define DEBUG_VAR(x)     ((void)0)
#else
    #define DEBUG_LOG(msg)   std::cout << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " | " << msg << std::endl
    #define DEBUG_VAR(x)     std::cout << "[DEBUG] " << #x " = " << (x) << "  (file:" << __FILE__ << " line:" << __LINE__ << ")" << std::endl
#endif

// ============================================================
//  第二招：日志分级 —— 不同级别输出不同信息
// ============================================================

enum class LogLevel {
    TRACE = 0,   // 最详细，跟踪每一步
    DEBUG = 1,   // 调试信息
    INFO  = 2,   // 一般信息
    WARN  = 3,   // 警告
    ERROR = 4    // 错误
};

// 当前日志级别（可以运行时调整）
inline LogLevel& currentLogLevel() {
    static LogLevel level = LogLevel::DEBUG;
    return level;
}

#define LOG(level, msg) do { \
    if (level >= currentLogLevel()) { \
        const char* levelNames[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"}; \
        std::cout << "[" << levelNames[static_cast<int>(level)] << "] " \
                  << msg << std::endl; \
    } \
} while(0)

#define LOG_TRACE(msg) LOG(LogLevel::TRACE, msg)
#define LOG_DEBUG(msg) LOG(LogLevel::DEBUG, msg)
#define LOG_INFO(msg)  LOG(LogLevel::INFO, msg)
#define LOG_WARN(msg)  LOG(LogLevel::WARN, msg)
#define LOG_ERROR(msg) LOG(LogLevel::ERROR, msg)

// ============================================================
//  第三招：函数进入退出跟踪 —— 看调用路径
// ============================================================

class FunctionTracer {
    std::string funcName_;
public:
    FunctionTracer(const char* func) : funcName_(func) {
        LOG_TRACE("→ 进入函数: " + funcName_);
    }
    ~FunctionTracer() {
        LOG_TRACE("← 退出函数: " + funcName_);
    }
};

// 用这个宏放在函数开头，就能自动跟踪函数的进入和退出
#define TRACE_FUNCTION() FunctionTracer _func_tracer_(__func__)

// ============================================================
//  练习1：找出求和函数的 bug
// ============================================================

// 这个函数有 bug，你能找出来吗？
// 提示：用 DEBUG_VAR 打印中间变量看看
int buggySum(int arr[], int size) {
    int sum = 0;
    // 这个版本只保留"漏掉 arr[0]"的 bug，便于安全地运行。
    // 经典错误写法还有 i <= size，那会越界读取 arr[size]（未定义行为），
    // 这里故意不那样写，避免演示程序自己踩内存。
    for (int i = 1; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

// 用调试输出找 bug 的正确姿势：
int debuggedSum(int arr[], int size) {
    DEBUG_LOG("开始计算数组和，size = " << size);
    int sum = 0;
    for (int i = 0; i < size; i++) {       // 已修复
        DEBUG_VAR(i);
        DEBUG_VAR(arr[i]);
        sum += arr[i];
        DEBUG_VAR(sum);
    }
    DEBUG_LOG("计算完成，sum = " << sum);
    return sum;
}

// ============================================================
//  练习2：字符串处理中的 bug
// ============================================================

// 目标：把字符串中的空格替换成 %20（URL编码）
// 这个实现有 bug，你找找看
// 提示：重点不是"越界"——std::string::operator[](size()) 是合法的，
// 返回 '\0'；这里的问题是循环多走了一次，结果末尾多了一个 '\0' 字符，
// result.size() 也比正确结果大 1。
std::string buggyUrlEncode(const std::string& input) {
    std::string result;
    for (size_t i = 0; i <= input.size(); i++) {  // ← 有问题
        if (input[i] == ' ') {
            result += "%20";
        } else {
            result += input[i];
        }
    }
    return result;
}

// 用日志定位问题
std::string debuggedUrlEncode(const std::string& input) {
    LOG_DEBUG("URL 编码开始，输入: '" << input << "', 长度: " << input.size());
    std::string result;
    for (size_t i = 0; i < input.size(); i++) {
        LOG_TRACE("  i=" << i << " char='" << input[i] << "' (" << static_cast<int>(input[i]) << ")");
        if (input[i] == ' ') {
            result += "%20";
            LOG_TRACE("    是空格，替换为 %20");
        } else {
            result += input[i];
        }
    }
    LOG_DEBUG("URL 编码完成，结果: '" << result << "'");
    return result;
}

// ============================================================
//  练习3：二分查找的 bug（经典面试题）
// ============================================================

// 在有序数组中查找 target，返回索引，找不到返回 -1
// 这个二分查找有 bug，很经典
int buggyBinarySearch(const std::vector<int>& arr, int target) {
    int left = 0;
    int right = arr.size() - 1;

    while (left < right) {  // ← 问题在这里！
        int mid = (left + right) / 2;  // 还有这里也有潜在问题
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid;  // ← 还有这里
        } else {
            right = mid;
        }
    }
    return -1;
}

// 正确的二分查找，加上调试输出
int debuggedBinarySearch(const std::vector<int>& arr, int target) {
    TRACE_FUNCTION();
    LOG_DEBUG("二分查找 target=" << target << "，数组大小=" << arr.size());

    int left = 0;
    int right = arr.size() - 1;

    while (left <= right) {  // 注意：是 <= 不是 <
        // 注意：left + right 可能溢出！用 left + (right - left) / 2 更安全
        int mid = left + (right - left) / 2;
        LOG_TRACE("  left=" << left << " mid=" << mid << " right=" << right
                  << " arr[mid]=" << arr[mid]);

        if (arr[mid] == target) {
            LOG_DEBUG("找到了！在位置 " << mid);
            return mid;
        } else if (arr[mid] < target) {
            LOG_TRACE("  arr[mid] < target，去右边找");
            left = mid + 1;  // 注意：是 mid+1 不是 mid
        } else {
            LOG_TRACE("  arr[mid] > target，去左边找");
            right = mid - 1;  // 注意：是 mid-1 不是 mid
        }
    }

    LOG_DEBUG("没找到，返回 -1");
    return -1;
}

// ============================================================
//  第四招：断言（assert）—— 假设检查
// ============================================================
//
//  assert 用于检查"应该永远成立"的条件
//  如果不成立，程序会直接崩溃（故意的！），告诉你哪里假设错了
//  发布版（定义 NDEBUG）下 assert 会被完全移除，不影响性能
//
//  什么时候用 assert：
//    - 检查程序内部的不变量（比如链表的大小应该等于节点数）
//    - 检查"不可能发生"的情况（比如 switch 的 default）
//    - 记录调用方必须满足的前置条件
//  注意：assert 不是给用户输入做校验的！发布版（定义 NDEBUG）会把 assert
//  完全移除，如果用它检查外部数据，非法输入就会一路放行。对来自用户/
//  文件/网络的数据，应该显式检查并返回错误码或抛出异常。

void processArray(int* arr, int size) {
    // 前置条件：这两个 assert 只表达"内部调用约定"，
    // 调用方必须保证；不能替代对外部输入的运行时校验。
    assert(arr != nullptr && "数组指针不能为空");
    assert(size > 0 && "数组大小必须大于0");

    // ... 处理 ...

    // 后置条件检查（假设处理后size应该不变）
    assert(size > 0 && "处理后大小应该仍大于0");
}

// ============================================================
//  第五招：二分法定位 bug —— 当你完全不知道 bug 在哪时
// ============================================================
//
//  思路：把代码切成两半，看看 bug 出现在前一半还是后一半
//  不断缩小范围，直到找到那一行
//
//  这是最朴素也最有效的方法，配合版本控制（git bisect）更好用

void demoBinaryDebugging() {
    std::cout << "\n=== 二分法调试思路 ===" << std::endl;
    std::cout << "假设一个大函数有 1000 行，程序崩溃了：" << std::endl;
    std::cout << "  第1步：在第 500 行打个输出，看看崩之前有没有走到这" << std::endl;
    std::cout << "  第2步：如果走到了，说明 bug 在 500-1000 行；" << std::endl;
    std::cout << "         如果没走到，说明在 0-500 行" << std::endl;
    std::cout << "  第3步：继续在可疑范围的中间打输出" << std::endl;
    std::cout << "  第4步：重复几次，很快就能定位到出问题的那一行" << std::endl;
    std::cout << "  这就是二分法，log2(1000) ≈ 10 步就能定位！" << std::endl;
}

// ============================================================
//  老师傅的话
// ============================================================
//
//  printf 调试看起来很土，但其实非常有效：
//
//  1. 不需要任何工具，任何环境都能用
//  2. 通常不会像调试器那样改变程序的控制流，但输出本身仍会影响时序和性能
//  3. 适合复现概率低的 bug（跑一晚上，看日志）
//
//  但它也有缺点：
//  1. 要改代码、重新编译
//  2. 输出太多了看花眼
//  3. 加输出本身可能影响时序（并发 bug 会消失）
//
//  进阶了之后要学调试器（GDB/VS调试器），但 printf 调试
//  永远是你的基本功，永远都有用。
//
//  调试的核心思路：
//  ✅ 先复现 bug —— 不能稳定复现的 bug 最难修
//  ✅ 缩小范围 —— 用二分法、注释掉一半代码
//  ✅ 提出假设 —— 你觉得 bug 可能是什么原因
//  ✅ 验证假设 —— 加日志/用调试器验证你的猜测
//  ✅ 修复并验证 —— 修了之后确认 bug 真的没了
//
//  小练习：
//  1. 试试把 buggySum 的 bug 找出来，用调试输出验证你的想法
//  2. 试试 buggyBinarySearch，写个测试用例触发它的 bug
//  3. 自己写一个有 bug 的函数，让同学/朋友来找 bug

int main() {
    std::cout << "========== 调试入门：printf 调试大法 ==========" << std::endl;

    // 演示日志分级
    std::cout << "\n--- 日志分级演示 ---" << std::endl;
    currentLogLevel() = LogLevel::TRACE;
    LOG_TRACE("这是 TRACE 级别的日志（最详细）");
    LOG_DEBUG("这是 DEBUG 级别的日志");
    LOG_INFO("这是 INFO 级别的日志");
    LOG_WARN("这是 WARN 级别的警告");
    LOG_ERROR("这是 ERROR 级别的错误");

    // 演示函数跟踪
    std::cout << "\n--- 函数跟踪演示 ---" << std::endl;
    currentLogLevel() = LogLevel::TRACE;
    debuggedUrlEncode("hello world");

    // 练习：找出 buggySum 的 bug
    std::cout << "\n--- 练习1：找出求和函数的 bug ---" << std::endl;
    int arr[] = {1, 2, 3, 4, 5};
    std::cout << "buggySum 的结果: " << buggySum(arr, 5) << std::endl;
    std::cout << "正确结果应该是: 15" << std::endl;
    std::cout << "（提示：数组下标从 0 开始）" << std::endl;

    // 练习：二分查找
    std::cout << "\n--- 练习2：二分查找的 bug ---" << std::endl;
    currentLogLevel() = LogLevel::DEBUG;
    std::vector<int> sortedArr = {1, 3, 5, 7, 9, 11, 13, 15};
    std::cout << "数组: [1, 3, 5, 7, 9, 11, 13, 15]" << std::endl;
    std::cout << "找 7: " << debuggedBinarySearch(sortedArr, 7)
              << " (应该是 3)" << std::endl;
    std::cout << "找 1: " << debuggedBinarySearch(sortedArr, 1)
              << " (应该是 0)" << std::endl;
    std::cout << "找 15: " << debuggedBinarySearch(sortedArr, 15)
              << " (应该是 7)" << std::endl;
    std::cout << "找 100: " << debuggedBinarySearch(sortedArr, 100)
              << " (应该是 -1)" << std::endl;

    // 二分法思路
    demoBinaryDebugging();

    // 断言演示
    std::cout << "\n--- 断言演示 ---" << std::endl;
    int testArr[] = {1, 2, 3};
    processArray(testArr, 3);
    std::cout << "断言检查通过，一切正常" << std::endl;

    // 试试取消下面这行的注释，看看断言失败是什么样的
    // processArray(nullptr, 5);

    std::cout << "\n========== 调试入门完成 ==========" << std::endl;
    std::cout << "记住：printf 调试是笨办法，但永远有效。" << std::endl;
    std::cout << "下一步：学调试器，比 printf 高效 10 倍！" << std::endl;

    return 0;
}
