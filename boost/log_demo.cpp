/**
 * @file log_demo.cpp
 * @brief Boost.Log 日志库深度解析
 *
 * Boost.Log 是 Boost 库中最常用的日志框架，工业级项目几乎必备。
 * 它提供了灵活的日志记录、过滤、格式化和输出能力。
 *
 * 主要内容：
 * 1. 日志基础：日志级别、简单日志记录
 * 2. 日志来源 (Logger)：全局 logger、多 logger、severity_logger
 * 3. 日志格式 (Formatter)：自定义输出格式
 * 4. 日志过滤 (Filter)：按级别、按来源过滤
 * 5. 日志接收器 (Sink)：控制台、文件、滚动文件、多 sink
 * 6. 日志属性 (Attribute)：时间戳、线程ID、进程ID等
 * 7. 异步日志
 * 8. 宽字符日志
 *
 * 老师傅的话：
 *   日志是项目的"黑匣子"。线上出了问题，全靠日志排查。
 *   一个好的日志系统要做到：级别清晰、信息完整、性能可靠。
 *   Boost.Log 功能强大，但配置也比较复杂，掌握核心用法就够了。
 *
 * 编译命令: g++ -std=c++17 log_demo.cpp -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lpthread
 *
 * 头文件：<boost/log/core.hpp>
 *        <boost/log/trivial.hpp>
 *        <boost/log/sources/severity_logger.hpp>
 *        <boost/log/sinks/sync_frontend.hpp>
 *        <boost/log/sinks/text_ostream_backend.hpp>
 *        <boost/log/expressions.hpp>
 *        <boost/log/utility/setup/file.hpp>
 *        <boost/log/utility/setup/console.hpp>
 *        <boost/log/utility/setup/common_attributes.hpp>
 */

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

// Boost.Log 头文件
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

// ============================================================================
// 1. 最简日志：BOOST_LOG_TRIVIAL 宏
// ============================================================================
// 这是 Boost.Log 最简单的用法，适合小型项目或快速上手。
// 日志级别：trace < debug < info < warning < error < fatal

void demo_trivial_logging() {
    std::cout << "\n===== 1. 最简日志（BOOST_LOG_TRIVIAL）=====\n";

    // 最简单的日志记录方式
    BOOST_LOG_TRIVIAL(trace)   << "这是 trace 级别的日志（最详细）";
    BOOST_LOG_TRIVIAL(debug)   << "这是 debug 级别的日志";
    BOOST_LOG_TRIVIAL(info)    << "这是 info 级别的日志（一般信息）";
    BOOST_LOG_TRIVIAL(warning) << "这是 warning 级别的日志（警告）";
    BOOST_LOG_TRIVIAL(error)   << "这是 error 级别的日志（错误）";
    BOOST_LOG_TRIVIAL(fatal)   << "这是 fatal 级别的日志（致命错误）";

    std::cout << "  默认情况下，所有级别都会输出到控制台\n";
    std::cout << "  可以通过设置过滤器只输出指定级别以上的日志\n";
}

// ============================================================================
// 2. 设置日志过滤级别
// ============================================================================
void set_log_level(logging::trivial::severity_level level) {
    logging::core::get()->set_filter(
        logging::trivial::severity >= level
    );
}

void demo_log_filter() {
    std::cout << "\n===== 2. 日志过滤（只显示 warning 及以上）=====\n";

    // 设置过滤器：只显示 warning 及以上级别
    set_log_level(logging::trivial::warning);

    BOOST_LOG_TRIVIAL(debug)   << "这条 debug 日志不会显示";
    BOOST_LOG_TRIVIAL(info)    << "这条 info 日志不会显示";
    BOOST_LOG_TRIVIAL(warning) << "这条 warning 日志会显示";
    BOOST_LOG_TRIVIAL(error)   << "这条 error 日志会显示";

    // 恢复默认（不过滤）
    logging::core::get()->reset_filter();
    std::cout << "  已恢复默认过滤级别\n";
}

// ============================================================================
// 3. severity_logger：更灵活的日志器
// ============================================================================
// BOOST_LOG_TRIVIAL 是全局单例的日志器。
// severity_logger 允许你创建多个独立的日志器实例。

// 自定义日志级别（可选，也可以用 boost::log::trivial::severity_level）
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// 为自定义级别实现 operator<<（用于日志输出）
template<typename CharT, typename TraitsT>
inline std::basic_ostream<CharT, TraitsT>&
operator<<(std::basic_ostream<CharT, TraitsT>& os, LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   os << "DEBUG";   break;
        case LogLevel::INFO:    os << "INFO";    break;
        case LogLevel::WARNING: os << "WARNING"; break;
        case LogLevel::ERROR:   os << "ERROR";   break;
        case LogLevel::FATAL:   os << "FATAL";   break;
        default:                os << "UNKNOWN"; break;
    }
    return os;
}

// 定义一个带级别的日志器类型
using MyLogger = src::severity_logger_mt<LogLevel>;

// 创建日志器实例（通常放在类里或全局）
MyLogger g_logger;

void demo_severity_logger() {
    std::cout << "\n===== 3. severity_logger 灵活日志器 =====\n";

    // 使用 severity_logger
    BOOST_LOG_SEV(g_logger, LogLevel::INFO)    << "应用程序启动";
    BOOST_LOG_SEV(g_logger, LogLevel::DEBUG)   << "调试信息：加载配置文件";
    BOOST_LOG_SEV(g_logger, LogLevel::WARNING) << "警告：配置项缺失，使用默认值";
    BOOST_LOG_SEV(g_logger, LogLevel::ERROR)   << "错误：数据库连接失败";

    // 可以有多个独立的 logger
    MyLogger db_logger;
    MyLogger net_logger;

    BOOST_LOG_SEV(db_logger, LogLevel::INFO)  << "[DB] 数据库连接池初始化";
    BOOST_LOG_SEV(net_logger, LogLevel::INFO) << "[NET] 网络模块初始化";

    std::cout << "  多个 logger 可以独立配置不同的 sink 和 filter\n";
}

// ============================================================================
// 4. 自定义日志格式
// ============================================================================
// 默认格式比较简单，可以自定义输出格式，添加时间戳、线程ID等信息。

void init_custom_format() {
    // 添加常用属性：时间戳、进程ID、线程ID
    logging::add_common_attributes();

    // 设置控制台输出格式
    logging::add_console_log(
        std::clog,
        keywords::format =
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>(
                       "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                << "] [" << expr::attr<unsigned int>("LineID")
                << "] [" << logging::trivial::severity
                << "] [" << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                << "] " << expr::smessage
    );
}

void demo_custom_format() {
    std::cout << "\n===== 4. 自定义日志格式 =====\n";
    std::cout << "  格式: [时间] [行号] [级别] [线程ID] 消息\n\n";

    init_custom_format();

    BOOST_LOG_TRIVIAL(info)  << "服务器启动成功";
    BOOST_LOG_TRIVIAL(debug) << "加载用户数据，共 1024 条";
    BOOST_LOG_TRIVIAL(warning) << "内存使用率超过 80%";
    BOOST_LOG_TRIVIAL(error) << "连接超时，正在重试...";

    std::cout << "\n  常用格式元素：\n";
    std::cout << "    %Y-%m-%d %H:%M:%S.%f  — 时间戳（毫秒精度）\n";
    std::cout << "    LineID                — 日志行号（递增）\n";
    std::cout << "    severity              — 日志级别\n";
    std::cout << "    ThreadID              — 线程ID\n";
    std::cout << "    ProcessID             — 进程ID\n";
    std::cout << "    smessage              — 日志消息内容\n";
}

// ============================================================================
// 5. 文件日志（滚动文件）
// ============================================================================
// 实际项目中，日志通常写到文件里，并且按大小或时间滚动。

void init_file_logging() {
    // 添加文件 sink：滚动日志文件
    // - 文件名格式：log_%Y%m%d_%H%M%S.log
    // - 每个文件最大 1MB
    // - 最多保留 5 个文件
    logging::add_file_log(
        keywords::file_name = "demo_%Y%m%d_%H%M%S.log",
        keywords::rotation_size = 1024 * 1024,  // 1MB 滚动
        keywords::max_files = 5,                 // 最多保留 5 个
        keywords::time_based_rotation =
            sinks::file::rotation_at_time_point(0, 0, 0),  // 每天零点滚动
        keywords::format =
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>(
                       "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                << "] [" << logging::trivial::severity
                << "] " << expr::smessage,
        keywords::auto_flush = true  // 自动刷新，防止崩溃丢失日志
    );
}

void demo_file_logging() {
    std::cout << "\n===== 5. 文件日志 =====\n";

    // 注意：为了不污染环境，这里只演示 API，不实际写很多文件
    // 实际项目中调用 init_file_logging() 即可

    BOOST_LOG_TRIVIAL(info)    << "这是写入文件的 info 日志";
    BOOST_LOG_TRIVIAL(debug)   << "这是写入文件的 debug 日志";
    BOOST_LOG_TRIVIAL(error)   << "这是写入文件的 error 日志";

    std::cout << "  日志已写入 demo_*.log 文件（如果启用了文件 sink）\n";
    std::cout << "  滚动策略：\n";
    std::cout << "    - rotation_size: 文件达到指定大小后滚动\n";
    std::cout << "    - rotation_at_time_point: 每天指定时间滚动\n";
    std::cout << "    - max_files: 最多保留的文件数（防止占满磁盘）\n";
    std::cout << "    - auto_flush: 自动刷新（性能略低，但安全）\n";
}

// ============================================================================
// 6. 多 Sink：同时输出到控制台和文件
// ============================================================================
void demo_multi_sink() {
    std::cout << "\n===== 6. 多 Sink 输出 =====\n";

    // Boost.Log 支持同时配置多个 sink
    // 比如：控制台只显示 warning 以上，文件记录所有日志
    // 或者：不同模块的日志写到不同文件

    BOOST_LOG_TRIVIAL(info)    << "这条日志会同时输出到多个 sink";
    BOOST_LOG_TRIVIAL(error)   << "错误日志会在所有 sink 中出现";

    std::cout << "  实际应用场景：\n";
    std::cout << "    - 控制台：只看 warning 和 error（开发调试）\n";
    std::cout << "    - 文件：记录所有级别（线上排查问题）\n";
    std::cout << "    - 远程：发送到日志服务器（集中收集分析）\n";
    std::cout << "    - 数据库：结构化日志存入 DB（统计分析）\n";
}

// ============================================================================
// 7. 命名作用域：记录函数进入退出
// ============================================================================
// BOOST_LOG_NAMED_SCOPE 可以记录函数的进入和退出，方便追踪调用栈。

void inner_function() {
    BOOST_LOG_NAMED_SCOPE("inner_function");
    BOOST_LOG_TRIVIAL(debug) << "执行内部逻辑";
}

void outer_function() {
    BOOST_LOG_NAMED_SCOPE("outer_function");
    BOOST_LOG_TRIVIAL(info) << "开始外层函数处理";
    inner_function();
    BOOST_LOG_TRIVIAL(info) << "外层函数处理完成";
}

void demo_named_scope() {
    std::cout << "\n===== 7. 命名作用域（函数追踪）=====\n";

    // 添加命名作用域属性
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

    // 重新设置格式，包含 Scope
    logging::add_console_log(
        std::clog,
        keywords::format =
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>(
                       "TimeStamp", "%H:%M:%S")
                << "] [" << logging::trivial::severity
                << "] "
                << expr::if_(expr::has_attr("Scope"))
                   [
                       expr::stream << "{"
                       << expr::format_named_scope(
                              "Scope",
                              keywords::format = "%n",
                              keywords::iteration = expr::reverse,
                              keywords::depth = 3)
                       << "} "
                   ]
                << expr::smessage
    );

    outer_function();

    std::cout << "  命名作用域可以追踪函数调用路径，排查问题时很有用\n";
    std::cout << "  类似手动打"进入函数"/"退出函数"日志，但更优雅\n";
}

// ============================================================================
// 8. 异步日志（高性能场景）
// ============================================================================
// 默认是同步日志（线程安全，调用时直接写入）。
// 异步日志：日志先写入队列，后台线程负责写出，性能更高。
// 适合高并发、低延迟的场景。

void demo_async_logging() {
    std::cout << "\n===== 8. 异步日志 =====\n";

    // Boost.Log 支持异步 sink（需要包含 text_file_backend 等）
    // 基本思路：
    //   - 前端（frontend）接收日志记录，放入队列
    //   - 后端（backend）在后台线程中实际写出
    //
    // 异步日志的优点：
    //   - 主线程不阻塞，性能更高
    //   - 适合高吞吐场景
    //
    // 异步日志的注意事项：
    //   - 程序崩溃可能丢失部分日志
    //   - 需要处理队列满的情况（丢弃/阻塞）
    //   - 退出时要确保队列排空

    BOOST_LOG_TRIVIAL(info) << "异步日志演示（配置需用 async_frontend）";

    std::cout << "  同步 vs 异步：\n";
    std::cout << "    同步：调用时直接写出，延迟低但阻塞\n";
    std::cout << "    异步：先入队列后台写出，吞吐高但有延迟\n";
    std::cout << "  选择建议：\n";
    std::cout << "    - 普通项目用同步日志就够了\n";
    std::cout << "    - 高性能服务才需要异步日志\n";
    std::cout << "    - 异步日志一定要配 auto_flush 和优雅关闭\n";
}

// ============================================================================
// 9. 日志性能对比与最佳实践
// ============================================================================
void demo_best_practices() {
    std::cout << "\n===== 9. 日志最佳实践 =====\n";

    std::cout << "\n  📌 级别使用规范：\n";
    std::cout << "    TRACE   - 最详细的追踪信息（函数调用、参数等），线上一般不开\n";
    std::cout << "    DEBUG   - 调试信息，开发阶段用，线上默认关闭\n";
    std::cout << "    INFO    - 正常运行信息（启动、停止、关键状态变化）\n";
    std::cout << "    WARNING - 警告（不影响运行，但需要关注）\n";
    std::cout << "    ERROR   - 错误（功能异常，但程序还能跑）\n";
    std::cout << "    FATAL   - 致命错误（程序无法继续运行）\n";

    std::cout << "\n  📌 日志内容要包含什么：\n";
    std::cout << "    - 什么时间发生的（时间戳）\n";
    std::cout << "    - 在哪个模块/函数发生的（scope/tag）\n";
    std::cout << "    - 发生了什么事（事件描述）\n";
    std::cout << "    - 相关的上下文数据（用户ID、请求ID等）\n";
    std::cout << "    - 错误码/异常信息（出错时）\n";

    std::cout << "\n  📌 性能注意事项：\n";
    std::cout << "    - 不要在循环里打大量 debug 日志\n";
    std::cout << "    - 复杂的日志格式化可以用 BOOST_LOG_STREAM_WITH_PARAMS\n";
    std::cout << "    - 高并发场景考虑异步日志\n";
    std::cout << "    - 日志级别用编译期宏也可以做零开销\n";

    std::cout << "\n  📌 安全注意事项：\n";
    std::cout << "    - 不要在日志里打密码、密钥、身份证号等敏感信息\n";
    std::cout << "    - 日志文件要设置合适的权限\n";
    std::cout << "    - 要限制日志文件大小，防止占满磁盘\n";

    BOOST_LOG_TRIVIAL(info) << "日志学习完成，记得在项目中善用日志";
}

// ============================================================================
// 老师傅的话
// ============================================================================
void laoshifu_says() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  👴 老师傅的话\n";
    std::cout << "============================================================\n";
    std::cout << "\n";
    std::cout << "  日志这东西，平时不起眼，出了问题就是救命稻草。\n";
    std::cout << "\n";
    std::cout << "  我见过太多项目，线上出了 bug，翻日志什么都查不到。\n";
    std::cout << "  要么是级别设错了，要么是信息不全，要么格式乱七八糟。\n";
    std::cout << "\n";
    std::cout << "  记住几条原则：\n";
    std::cout << "  1️⃣  INFO 级别要能看出程序的主要运行轨迹\n";
    std::cout << "  2️⃣  出错时的日志一定要包含足够的上下文信息\n";
    std::cout << "  3️⃣  日志格式要统一，方便用脚本分析\n";
    std::cout << "  4️⃣  生产环境默认 INFO 级别，出问题才开 DEBUG\n";
    std::cout << "  5️⃣  敏感信息绝对不能打日志\n";
    std::cout << "\n";
    std::cout << "  Boost.Log 功能很全，但核心就那几个概念：\n";
    std::cout << "  Logger（日志器）→ Filter（过滤器）→ Formatter（格式化器）→ Sink（接收器）\n";
    std::cout << "  理解了这条流水线，Boost.Log 就入门了。\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 小练习
// ============================================================================
void exercises() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  📝 小练习\n";
    std::cout << "============================================================\n";
    std::cout << "\n";
    std::cout << "  1. 配置一个日志系统：\n";
    std::cout << "     - 控制台输出 warning 以上级别\n";
    std::cout << "     - 文件输出所有级别（滚动文件，最大 5 个）\n";
    std::cout << "     - 格式包含：时间、级别、线程ID、消息\n";
    std::cout << "\n";
    std::cout << "  2. 给你之前写的学生管理系统加上日志：\n";
    std::cout << "     - 增删改查操作都打 info 日志\n";
    std::cout << "     - 错误情况打 error 日志（比如删除不存在的学生）\n";
    std::cout << "     - 函数入口出口打 debug 日志\n";
    std::cout << "\n";
    std::cout << "  3. 思考：什么时候用同步日志，什么时候用异步日志？\n";
    std::cout << "     写一段你自己的理解。\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  Boost.Log 日志库深度解析\n";
    std::cout << "  工业级日志系统 —— 项目必备技能\n";
    std::cout << "============================================================\n";

    // 初始化（实际项目中一般在 main 开头调用一次）
    logging::add_console_log(std::cout);
    logging::add_common_attributes();

    demo_trivial_logging();
    demo_log_filter();
    demo_severity_logger();

    // 重新初始化格式
    logging::core::get()->remove_all_sinks();
    demo_custom_format();

    // 文件日志（可选）
    // init_file_logging();
    demo_file_logging();

    demo_multi_sink();
    demo_named_scope();
    demo_async_logging();
    demo_best_practices();

    laoshifu_says();
    exercises();

    return 0;
}
