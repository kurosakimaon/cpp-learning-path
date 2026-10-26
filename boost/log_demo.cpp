/**
 * @file log_demo.cpp
 * @brief Boost.Log 日志库深度解析
 *
 * Boost.Log 是 Boost 库中最常用的日志框架，工业级项目几乎必备。
 * 它提供了灵活的日志记录、过滤、格式化和输出能力。
 *
 * 主要内容：
 * 1. 最简日志：BOOST_LOG_TRIVIAL
 * 2. 日志过滤 (Filter)：按级别过滤
 * 3. severity_logger：自定义级别、多 logger
 * 4. 日志格式 (Formatter)：时间戳、记录序号、线程ID
 * 5. 文件日志：滚动文件、自动刷新
 * 6. 多 Sink：控制台 + 文件 + 远程
 * 7. 命名作用域：记录函数调用路径
 * 8. 异步日志：asynchronous_sink
 * 9. 最佳实践
 *
 * 老师傅的话：
 *   日志是项目的"黑匣子"。线上出了问题，全靠日志排查。
 *   一个好的日志系统要做到：级别清晰、信息完整、性能可靠。
 *   Boost.Log 功能强大，但配置也比较复杂，掌握核心用法就够了。
 *
 * 编译命令（动态链接 Boost.Log 时必须定义 BOOST_LOG_DYN_LINK）:
 *   Linux:        g++ -std=c++17 -DBOOST_LOG_DYN_LINK log_demo.cpp \
 *                     -lboost_log -lboost_log_setup -lboost_thread -lboost_filesystem -lpthread
 *   MinGW/MSYS2:  g++ -std=c++17 -DBOOST_LOG_DYN_LINK log_demo.cpp \
 *                     -lboost_log-mt -lboost_log_setup-mt -lboost_thread-mt \
 *                     -lboost_filesystem-mt -lboost_chrono-mt -lboost_atomic-mt
 *   （Windows 预编译库常带 -mt 后缀，以实际库文件名为准）
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
#include <boost/log/sinks/async_frontend.hpp>
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
#include <boost/core/null_deleter.hpp>
#include <boost/make_shared.hpp>

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

    std::cout << "  多个 logger 共享 core 的 sink 和 filter；要区分来源，"
                 "可以在日志记录上附加 channel 等属性再过滤\n";
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
    std::cout << "  格式: [时间] [记录序号] [级别] [线程ID] 消息\n\n";

    init_custom_format();

    BOOST_LOG_TRIVIAL(info)  << "服务器启动成功";
    BOOST_LOG_TRIVIAL(debug) << "加载用户数据，共 1024 条";
    BOOST_LOG_TRIVIAL(warning) << "内存使用率超过 80%";
    BOOST_LOG_TRIVIAL(error) << "连接超时，正在重试...";

    std::cout << "\n  常用格式元素：\n";
    std::cout << "    %Y-%m-%d %H:%M:%S.%f  — 时间戳（小数秒，微秒精度）\n";
    std::cout << "    LineID                — 日志记录序号（递增计数器，不是源码行号）\n";
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
    // - 文件名格式：demo_%Y%m%d_%H%M%S.log
    // - 每个文件最大 1MB
    // - 每天零点滚动
    // 注意：max_files 这类“保留几个文件”的限制要配合
    // target（目标目录/文件收集器）才会生效，本示例没有启用收集器。
    logging::add_file_log(
        keywords::file_name = "demo_%Y%m%d_%H%M%S.log",
        keywords::rotation_size = 1024 * 1024,  // 1MB 滚动
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

    init_file_logging();

    BOOST_LOG_TRIVIAL(info)    << "这是写入文件的 info 日志";
    BOOST_LOG_TRIVIAL(debug)   << "这是写入文件的 debug 日志";
    BOOST_LOG_TRIVIAL(error)   << "这是写入文件的 error 日志";
    logging::core::get()->flush();

    std::cout << "  日志已写入当前目录的 demo_*.log 文件\n";
    std::cout << "  滚动策略：\n";
    std::cout << "    - rotation_size: 文件达到指定大小后滚动\n";
    std::cout << "    - rotation_at_time_point: 每天指定时间滚动\n";
    std::cout << "    - max_files: 只在配置了 target（文件收集器）时生效\n";
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

    // 先移除旧 sink，否则会同时往两个控制台 sink 写，日志重复出现
    logging::core::get()->remove_all_sinks();

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
    std::cout << "  类似手动打“进入函数”/“退出函数”日志，但更优雅\n";
}

// ============================================================================
// 8. 异步日志（高性能场景）
// ============================================================================
// 默认是同步日志（线程安全，调用时直接写入）。
// 异步日志：日志先写入队列，后台线程负责写出，性能更高。
// 适合高并发、低延迟的场景。

void demo_async_logging() {
    std::cout << "\n===== 8. 异步日志 =====\n";

    // 异步 sink = 前端（队列 + 后台线程）+ 后端（真正写出的 sink）
    // 这里后端仍用 text_ostream_backend 输出到控制台，便于观察。
    logging::core::get()->remove_all_sinks();

    using async_sink_t = sinks::asynchronous_sink<sinks::text_ostream_backend>;
    auto backend = boost::make_shared<sinks::text_ostream_backend>();
    backend->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
    backend->auto_flush(true);

    auto async_sink = boost::make_shared<async_sink_t>(backend);
    async_sink->set_formatter(
        expr::stream << "[" << logging::trivial::severity << "] "
                     << expr::smessage);
    logging::core::get()->add_sink(async_sink);

    BOOST_LOG_TRIVIAL(info) << "异步日志演示：这条日志由后台线程写出";

    // flush() 会等队列里的日志真正写完，退出前必须调用。
    // sink 继续挂在 core 上，后续日志仍走异步；进程退出时 core 析构会 stop。
    async_sink->flush();

    std::cout << "  同步 vs 异步：\n";
    std::cout << "    同步：调用时直接写出，延迟低但阻塞\n";
    std::cout << "    异步：先入队列，后台线程写出，吞吐高但有延迟\n";
    std::cout << "  注意事项：\n";
    std::cout << "    - 程序崩溃可能丢失队列中尚未写出的日志\n";
    std::cout << "    - 退出前要 flush 并停止后台线程（sink 析构会自动 stop）\n";
    std::cout << "    - 队列满时的策略（阻塞/丢弃）要提前想清楚\n";
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
    std::cout << "    - 需要给单条日志附加参数（级别、channel、自定义属性）时，"
                 "用 BOOST_LOG_STREAM_WITH_PARAMS\n";
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
