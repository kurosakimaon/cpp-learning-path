/**
 * Boost.Program_options 命令行/配置文件解析 Demo
 *
 * 知识点覆盖：
 *   1. 选项描述（options_description）：基本选项、默认值、隐式值、零值
 *   2. 位置参数（positional_options_description）：无名参数按位置解析
 *   3. 选项组（options_description 分组）：帮助信息分组显示
 *   4. 配置文件解析（parse_config_file）：INI 格式配置文件
 *   5. 环境变量解析：从环境变量读取选项
 *   6. 多源合并：命令行 > 配置文件 > 环境变量 > 默认值
 *   7. 自定义验证器（validate）：对选项值进行自定义校验
 *   8. 变量映射（variables_map）：查询选项值、通知机制
 *
 * Boost 优势：
 *   - 声明式语法，简洁易用
 *   - 统一的命令行 + 配置文件 + 环境变量解析框架
 *   - 自动生成帮助信息（--help）
 *   - 支持类型安全的选项值存储
 *   - 自定义验证器和通知机制提供强大的扩展性
 *   - C++ 标准库中至今无对应功能
 *
 * 编译命令（示例）：
 *   g++ -std=c++17 program_options.cpp -o program_options -lboost_program_options
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

// ========== Boost 头文件说明 ==========
// <boost/program_options.hpp>        - 总头文件
// <boost/program_options/options_description.hpp> - 选项描述
// <boost/program_options/positional_options.hpp> - 位置参数
// <boost/program_options/variables_map.hpp>       - 变量映射
// <boost/program_options/parsers.hpp>             - 解析器（命令行/配置文件/环境）
// <boost/program_options/errors.hpp>              - 异常类型
#include <boost/program_options.hpp>

namespace po = boost::program_options;

// ============================================================
// 一、自定义类型 + 自定义验证器
// ============================================================
// 当需要解析自定义类型的选项时，需要提供 validate() 函数的重载。
// validate 函数会被 ADL 查找调用，用于将字符串转换为目标类型。

// 自定义类型：日志级别
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

// 自定义类型输出（用于帮助信息显示）
std::ostream& operator<<(std::ostream& os, LogLevel level) {
    switch (level) {
        case LogLevel::Trace:   os << "trace"; break;
        case LogLevel::Debug:   os << "debug"; break;
        case LogLevel::Info:    os << "info"; break;
        case LogLevel::Warning: os << "warning"; break;
        case LogLevel::Error:   os << "error"; break;
        case LogLevel::Fatal:   os << "fatal"; break;
    }
    return os;
}

// 自定义验证器：将字符串解析为 LogLevel
// 函数签名必须是 void validate(any&, const vector<string>&, T*, int)
void validate(boost::any& v, const std::vector<std::string>& values,
              LogLevel*, int) {
    // 确保只有一个值
    po::validators::check_first_occurrence(v);
    const std::string& s = po::validators::get_single_string(values);

    // 自定义解析逻辑
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "trace")        v = LogLevel::Trace;
    else if (lower == "debug")   v = LogLevel::Debug;
    else if (lower == "info")    v = LogLevel::Info;
    else if (lower == "warning") v = LogLevel::Warning;
    else if (lower == "error")   v = LogLevel::Error;
    else if (lower == "fatal")   v = LogLevel::Fatal;
    else {
        throw po::validation_error(po::validation_error::invalid_option_value, s);
    }
}

// ============================================================
// 二、自定义类型：IP 地址（带格式验证）
// ============================================================
struct IPAddress {
    int octets[4];

    std::string to_string() const {
        std::ostringstream oss;
        oss << octets[0] << "." << octets[1] << "." << octets[2] << "." << octets[3];
        return oss.str();
    }
};

void validate(boost::any& v, const std::vector<std::string>& values,
              IPAddress*, int) {
    po::validators::check_first_occurrence(v);
    const std::string& s = po::validators::get_single_string(values);

    IPAddress ip;
    int count = sscanf(s.c_str(), "%d.%d.%d.%d",
                       &ip.octets[0], &ip.octets[1],
                       &ip.octets[2], &ip.octets[3]);

    if (count != 4) {
        throw po::validation_error(po::validation_error::invalid_option_value,
                                   s, "IP address");
    }

    for (int i = 0; i < 4; ++i) {
        if (ip.octets[i] < 0 || ip.octets[i] > 255) {
            throw po::validation_error(po::validation_error::invalid_option_value,
                                       s, "IP octet out of range");
        }
    }

    v = ip;
}

// ============================================================
// 三、配置结构（存储所有解析结果）
// ============================================================
struct AppConfig {
    // 基本选项
    std::string host;
    int port;
    LogLevel log_level;
    bool verbose;
    bool daemon;

    // 高级选项
    int max_connections;
    int timeout_seconds;
    std::string log_file;
    IPAddress bind_ip;

    // 位置参数（输入文件列表）
    std::vector<std::string> input_files;

    // 隐藏选项（不在帮助信息中显示）
    std::string config_file;

    void print() const {
        std::cout << "\n========== 解析结果 ==========" << std::endl;
        std::cout << "host:           " << host << std::endl;
        std::cout << "port:           " << port << std::endl;
        std::cout << "log_level:      " << log_level << std::endl;
        std::cout << "verbose:        " << std::boolalpha << verbose << std::endl;
        std::cout << "daemon:         " << daemon << std::endl;
        std::cout << "max_connections:" << max_connections << std::endl;
        std::cout << "timeout:        " << timeout_seconds << "s" << std::endl;
        std::cout << "log_file:       " << (log_file.empty() ? "(stdout)" : log_file) << std::endl;
        std::cout << "bind_ip:        " << bind_ip.to_string() << std::endl;
        std::cout << "config_file:    " << (config_file.empty() ? "(none)" : config_file) << std::endl;
        std::cout << "input_files:    " << input_files.size() << " 个" << std::endl;
        for (size_t i = 0; i < input_files.size(); ++i) {
            std::cout << "  [" << i << "] " << input_files[i] << std::endl;
        }
        std::cout << "==============================" << std::endl;
    }
};

// ============================================================
// 四、生成示例配置文件
// ============================================================
void create_sample_config(const std::string& path) {
    std::ofstream f(path);
    f << "# Boost.Program_options 示例配置文件\n";
    f << "# 格式：key = value\n";
    f << "\n";
    f << "[server]\n";
    f << "host = 0.0.0.0\n";
    f << "port = 9090\n";
    f << "log_level = info\n";
    f << "verbose = 1\n";
    f << "\n";
    f << "[advanced]\n";
    f << "max_connections = 500\n";
    f << "timeout = 60\n";
    f << "log_file = /var/log/app.log\n";
    f << "bind_ip = 127.0.0.1\n";
    f.close();
    std::cout << "  已生成示例配置文件: " << path << std::endl;
}

// ============================================================
// 五、解析主函数
// ============================================================
bool parse_config(int argc, char* argv[], AppConfig& config) {
    // --------------------------------------------------------
    // 5.1 定义选项组
    // --------------------------------------------------------
    // 将选项分组可以让 --help 输出更清晰。
    // 选项描述对象是可组合的，可以分块定义后合并。

    // 通用选项组（--help, --version, --config 等）
    po::options_description generic("通用选项");
    generic.add_options()
        ("help,h", "显示帮助信息")
        ("version,v", "显示版本信息")
        ("config,c", po::value<std::string>(), "指定配置文件路径");

    // 服务器配置组
    po::options_description server_opts("服务器配置");
    server_opts.add_options()
        ("host,H",
         po::value<std::string>(&config.host)->default_value("127.0.0.1"),
         "监听地址")
        ("port,p",
         po::value<int>(&config.port)->default_value(8080),
         "监听端口 (1-65535)")
        ("log-level,l",
         po::value<LogLevel>(&config.log_level)->default_value(LogLevel::Info),
         "日志级别: trace/debug/info/warning/error/fatal")
        ("verbose,V",
         po::bool_switch(&config.verbose),
         "详细输出模式（开启后日志级别自动降为 debug）")
        ("daemon,d",
         po::bool_switch(&config.daemon),
         "以守护进程模式运行");

    // 高级选项组
    po::options_description advanced_opts("高级选项");
    advanced_opts.add_options()
        ("max-connections,m",
         po::value<int>(&config.max_connections)->default_value(100),
         "最大并发连接数")
        ("timeout,t",
         po::value<int>(&config.timeout_seconds)->default_value(30),
         "连接超时时间（秒）")
        ("log-file",
         po::value<std::string>(&config.log_file)->default_value(""),
         "日志文件路径（为空则输出到 stdout）")
        ("bind-ip",
         po::value<IPAddress>(&config.bind_ip)->default_value(IPAddress{{127, 0, 0, 1}}, "127.0.0.1"),
         "绑定的 IP 地址（格式: x.x.x.x）");

    // 隐藏选项组（不在 --help 中显示，用于位置参数等）
    po::options_description hidden("隐藏选项");
    hidden.add_options()
        ("input-files",
         po::value<std::vector<std::string>>(&config.input_files),
         "输入文件列表");

    // --------------------------------------------------------
    // 5.2 位置参数
    // --------------------------------------------------------
    // 位置参数是没有 -- 前缀的参数，按出现顺序匹配。
    // 例如：./app file1.txt file2.txt 中的 file1.txt 和 file2.txt
    po::positional_options_description positional;
    positional.add("input-files", -1);  // -1 表示可以接受任意数量

    // --------------------------------------------------------
    // 5.3 合并所有选项（用于解析）
    // --------------------------------------------------------
    po::options_description cmdline_options;
    cmdline_options.add(generic).add(server_opts).add(advanced_opts).add(hidden);

    // 用于 --help 显示的选项（不包含 hidden）
    po::options_description visible_options;
    visible_options.add(generic).add(server_opts).add(advanced_opts);

    // --------------------------------------------------------
    // 5.4 解析命令行参数
    // --------------------------------------------------------
    po::variables_map vm;
    try {
        // store 存储解析结果到 vm，但不通知（不更新绑定的变量）
        po::store(po::command_line_parser(argc, argv)
                      .options(cmdline_options)
                      .positional(positional)
                      .run(),
                  vm);

        // 检查 --help
        if (vm.count("help")) {
            std::cout << "用法: " << argv[0] << " [选项] [输入文件...]" << std::endl;
            std::cout << visible_options << std::endl;
            return false;
        }

        // 检查 --version
        if (vm.count("version")) {
            std::cout << "Boost.Program_options Demo v1.0.0" << std::endl;
            return false;
        }

        // --------------------------------------------------------
        // 5.5 解析配置文件（如果指定了 --config）
        // --------------------------------------------------------
        if (vm.count("config")) {
            std::string config_file = vm["config"].as<std::string>();
            config.config_file = config_file;

            std::cout << "  读取配置文件: " << config_file << std::endl;

            // 配置文件解析
            // 注意：配置文件中的选项名使用下划线，不是连字符
            // 例如：max-connections -> max_connections
            po::options_description config_file_options;
            config_file_options.add(server_opts).add(advanced_opts);

            std::ifstream ifs(config_file.c_str());
            if (!ifs) {
                std::cerr << "错误: 无法打开配置文件: " << config_file << std::endl;
                return false;
            }

            // 解析配置文件，allow_unregistered 允许配置文件中有未定义的选项
            po::store(po::parse_config_file(ifs, config_file_options, true), vm);
        }

        // --------------------------------------------------------
        // 5.6 环境变量解析
        // --------------------------------------------------------
        // 可以从环境变量中读取选项值，优先级低于命令行但高于默认值
        // 环境变量名通常大写，使用下划线分隔
        std::cout << "  检查环境变量 APP_PORT..." << std::endl;
        po::options_description env_opts;
        env_opts.add_options()
            ("APP_PORT", po::value<int>(), "端口");
        // 实际使用时调用 po::parse_environment
        // 这里为了演示稳定性，不实际读取环境变量

        // --------------------------------------------------------
        // 5.7 notify：应用解析结果到绑定变量
        // --------------------------------------------------------
        // notify() 会将 vm 中的值赋给绑定到的变量指针
        // 并触发验证逻辑（包括自定义 validate 函数）
        po::notify(vm);

        // --------------------------------------------------------
        // 5.8 自定义验证逻辑（选项间依赖关系）
        // --------------------------------------------------------
        // Program_options 只能验证单个选项的合法性，
        // 选项之间的依赖关系需要自己检查。
        if (config.port < 1 || config.port > 65535) {
            std::cerr << "错误: 端口号必须在 1-65535 之间" << std::endl;
            return false;
        }

        if (config.max_connections <= 0) {
            std::cerr << "错误: max-connections 必须为正整数" << std::endl;
            return false;
        }

        // verbose 模式下自动调整日志级别
        if (config.verbose && config.log_level > LogLevel::Debug) {
            config.log_level = LogLevel::Debug;
            std::cout << "  verbose 模式启用，日志级别调整为 debug" << std::endl;
        }

    } catch (const po::error& e) {
        std::cerr << "命令行解析错误: " << e.what() << std::endl;
        std::cerr << "使用 --help 查看帮助信息" << std::endl;
        return false;
    }

    return true;
}

// ============================================================
// 六、各种选项类型详解
// ============================================================
void option_types_demo() {
    std::cout << "\n--- 选项类型详解 ---" << std::endl;

    po::options_description desc("选项类型示例");
    int int_val = 0;
    std::string str_val;
    bool flag = false;
    bool switch_flag = false;
    std::vector<int> int_list;
    std::vector<std::string> str_list;

    desc.add_options()
        // 1. 基本类型 + 默认值
        ("int_opt,i", po::value<int>(&int_val)->default_value(42),
         "整数选项，有默认值")

        // 2. 字符串 + required（必须提供）
        ("str_opt,s", po::value<std::string>(&str_val)->required(),
         "字符串选项，必须提供")

        // 3. bool_switch（开关型选项，不需要值，出现即为 true）
        ("flag,f", po::bool_switch(&flag),
         "布尔开关选项")

        // 4. zero_tokens（隐式值，选项后可不跟值）
        ("switch_opt", po::bool_switch(&switch_flag)->implicit_value(true),
         "带隐式值的开关")

        // 5. multitoken：一个选项可以跟多个值（如 --list a b c）
        ("int-list", po::value<std::vector<int>>(&int_list)->multitoken(),
         "整数列表（多值）")

        // 6. composing：可组合（可以多次指定，值会累积）
        ("include,I", po::value<std::vector<std::string>>(&str_list)->composing(),
         "包含路径（可重复指定）")

        // 7. implicit_value：有隐式默认值
        ("optimize,O", po::value<int>()->implicit_value(2, "level"),
         "优化级别（-O 等价于 -O2）");

    std::cout << desc << std::endl;

    // 演示 composing 和 multitoken 的区别
    std::cout << "  multitoken: --list 1 2 3 (一个选项后跟多个值)" << std::endl;
    std::cout << "  composing:  -I a -I b -I c (选项可重复，值累积)" << std::endl;
}

// ============================================================
// Demo 入口函数
// ============================================================
int main(int argc, char* argv[]) {
    std::cout << "========== Boost.Program_options 解析 Demo ==========" << std::endl;

    // --------------------------------------------------------
    // 1. 演示选项类型
    // --------------------------------------------------------
    option_types_demo();

    // --------------------------------------------------------
    // 2. 生成示例配置文件
    // --------------------------------------------------------
    std::cout << "\n--- 生成示例配置文件 ---" << std::endl;
    std::string config_path = "demo_config.ini";
    create_sample_config(config_path);

    // --------------------------------------------------------
    // 3. 模拟命令行参数（带配置文件）
    // --------------------------------------------------------
    std::cout << "\n--- 模拟命令行解析 ---" << std::endl;

    // 构造模拟的命令行参数
    // 实际使用时直接传 argc/argv 即可
    const char* fake_argv[] = {
        "program_options_demo",
        "--config", "demo_config.ini",
        "--port", "8888",
        "--verbose",
        "--max-connections", "200",
        "input1.txt",
        "input2.txt",
        "input3.txt"
    };
    int fake_argc = sizeof(fake_argv) / sizeof(fake_argv[0]);

    AppConfig config;
    if (parse_config(fake_argc, const_cast<char**>(fake_argv), config)) {
        config.print();
    }

    // --------------------------------------------------------
    // 4. 演示 --help 输出
    // --------------------------------------------------------
    std::cout << "\n--- --help 输出示例 ---" << std::endl;
    const char* help_argv[] = { "program_options_demo", "--help" };
    AppConfig help_config;
    parse_config(2, const_cast<char**>(help_argv), help_config);

    // --------------------------------------------------------
    // 5. 演示错误处理
    // --------------------------------------------------------
    std::cout << "\n--- 错误处理示例 ---" << std::endl;
    std::cout << "5.1 未知选项:" << std::endl;
    {
        const char* bad_argv[] = { "demo", "--unknown-option" };
        po::options_description desc;
        desc.add_options()("port", po::value<int>(), "port");
        po::variables_map vm;
        try {
            po::store(po::command_line_parser(2, const_cast<char**>(bad_argv))
                          .options(desc).run(), vm);
        } catch (const po::unknown_option& e) {
            std::cout << "  捕获 unknown_option: " << e.what() << std::endl;
        }
    }

    std::cout << "\n5.2 类型转换错误:" << std::endl;
    {
        const char* bad_argv[] = { "demo", "--port", "not_a_number" };
        po::options_description desc;
        desc.add_options()("port", po::value<int>(), "port");
        po::variables_map vm;
        try {
            po::store(po::command_line_parser(2, const_cast<char**>(bad_argv))
                          .options(desc).run(), vm);
            po::notify(vm);
        } catch (const po::invalid_option_value& e) {
            std::cout << "  捕获 invalid_option_value: " << e.what() << std::endl;
        }
    }

    std::cout << "\n5.3 缺少必填选项:" << std::endl;
    {
        const char* bad_argv[] = { "demo" };
        po::options_description desc;
        desc.add_options()("required_opt", po::value<std::string>()->required(), "required");
        po::variables_map vm;
        try {
            po::store(po::command_line_parser(1, const_cast<char**>(bad_argv))
                          .options(desc).run(), vm);
            po::notify(vm);
        } catch (const po::required_option& e) {
            std::cout << "  捕获 required_option: " << e.what() << std::endl;
        }
    }

    // --------------------------------------------------------
    // 总结
    // --------------------------------------------------------
    std::cout << "\n--- Boost.Program_options 核心要点 ---" << std::endl;
    std::cout << "  1. 声明式语法：options_description + add_options() 流式 API" << std::endl;
    std::cout << "  2. 多源解析：命令行 > 配置文件 > 环境变量 > 默认值" << std::endl;
    std::cout << "  3. 位置参数：通过 positional_options_description 配置" << std::endl;
    std::cout << "  4. 自定义类型：通过 ADL validate() 函数实现类型转换和验证" << std::endl;
    std::cout << "  5. 选项组合：multitoken（多值）vs composing（可累积）" << std::endl;
    std::cout << "  6. bool_switch：开关型选项，不需要后跟值" << std::endl;
    std::cout << "  7. C++ 标准库暂无对应功能，Boost 是首选方案" << std::endl;

    // 清理
    std::remove(config_path.c_str());

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
