/**
 * @file property_tree_demo.cpp
 * @brief Boost.PropertyTree 配置管理深度解析
 *
 * Boost.PropertyTree 是一个轻量级的配置文件读写库，
 * 支持 INI、JSON、XML、INFO 四种格式，非常适合项目配置管理。
 *
 * 核心思想：用树形结构存储键值对，支持嵌套和数组。
 * 就像一棵倒挂的树，每个节点可以有子节点，也可以有值。
 *
 * 主要内容：
 * 1. 什么是 PropertyTree：树形数据结构
 * 2. INI 配置文件读写（最常用）
 * 3. JSON 配置文件读写
 * 4. XML 配置文件读写
 * 5. 遍历和查找
 * 6. 数据转换（get 与 get_child）
 * 7. 默认值与异常处理
 * 8. 实际项目：完整的应用配置类
 *
 * 老师傅的话：
 *   配置文件是每个项目都有的东西。硬编码参数是大忌！
 *   把可变的东西放到配置文件里，代码才灵活。
 *   PropertyTree 用法简单，功能够用，中小项目首选。
 *   大项目可以考虑专门的配置中心，但原理是相通的。
 *
 * 编译命令: g++ -std=c++17 property_tree_demo.cpp
 *   （PropertyTree 的这几个解析器都是头文件实现，不需要额外链接库）
 *
 * 头文件：<boost/property_tree/ptree.hpp>
 *        <boost/property_tree/ini_parser.hpp>
 *        <boost/property_tree/json_parser.hpp>
 *        <boost/property_tree/xml_parser.hpp>
 *        <boost/property_tree/info_parser.hpp>
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>

// PropertyTree 头文件
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace pt = boost::property_tree;

// ============================================================================
// 1. PropertyTree 基础：创建和访问
// ============================================================================
// ptree 本质上是一个 key-value 的树形结构
// 路径用点号分隔，例如 "database.host"

void demo_basic_ptree() {
    std::cout << "\n===== 1. PropertyTree 基础 =====\n";

    // 创建一个空的 ptree
    pt::ptree root;

    // 写入数据（put 会覆盖已有值）
    root.put("app.name", "MyApp");
    root.put("app.version", "1.0.0");
    root.put("app.debug", true);

    root.put("database.host", "localhost");
    root.put("database.port", 3306);
    root.put("database.user", "admin");
    root.put("database.password", "123456");  // 实际项目别写明文密码！

    root.put("log.level", "info");
    root.put("log.file_path", "/var/log/myapp.log");
    root.put("log.max_size_mb", 100);

    // 读取数据
    std::string app_name = root.get<std::string>("app.name");
    int db_port = root.get<int>("database.port");
    bool debug_mode = root.get<bool>("app.debug");

    std::cout << "应用名: " << app_name << "\n";
    std::cout << "版本号: " << root.get<std::string>("app.version") << "\n";
    std::cout << "调试模式: " << std::boolalpha << debug_mode << "\n";
    std::cout << "数据库端口: " << db_port << "\n";
    std::cout << "日志级别: " << root.get<std::string>("log.level") << "\n";

    // 带默认值的读取（key 不存在时返回默认值，不抛异常）
    int timeout = root.get<int>("database.timeout", 30);
    std::cout << "超时时间（默认值）: " << timeout << " 秒\n";

    // get_optional：返回 optional，自己判断是否存在
    boost::optional<std::string> proxy = root.get_optional<std::string>("proxy.server");
    if (proxy) {
        std::cout << "代理服务器: " << *proxy << "\n";
    } else {
        std::cout << "代理服务器: 未配置\n";
    }

    std::cout << "\n  路径用点号分隔，就像访问对象的属性一样\n";
    std::cout << "  三种读取方式：get(必填) / get(默认值) / get_optional(可选)\n";
}

// ============================================================================
// 2. INI 文件读写
// ============================================================================
// INI 是最经典的配置文件格式，结构简单，可读性好。
// 适合层级不深的配置（最多两级：section + key）

void demo_ini_file() {
    std::cout << "\n===== 2. INI 配置文件 =====\n";

    // 生成一个示例 INI 文件（实际项目中是手工编写的）
    const char* ini_content = R"(
; 应用配置
[app]
name = MyApplication
version = 2.1.0
debug = false

; 数据库配置
[database]
host = 192.168.1.100
port = 5432
user = postgres
password = secret
max_connections = 100
connection_timeout = 30

; 日志配置
[log]
level = warning
file = /var/log/app.log
max_size_mb = 50
rotate_count = 10
)";

    // 写入临时文件
    std::ofstream("config.ini") << ini_content;
    std::cout << "  已生成示例配置文件 config.ini\n\n";

    // 读取 INI 文件
    pt::ptree tree;
    try {
        pt::read_ini("config.ini", tree);
    } catch (const pt::ini_parser_error& e) {
        std::cerr << "读取 INI 文件失败: " << e.what() << "\n";
        return;
    }

    // 读取配置
    std::cout << "  读取配置：\n";
    std::cout << "    应用名: " << tree.get<std::string>("app.name") << "\n";
    std::cout << "    版本:   " << tree.get<std::string>("app.version") << "\n";
    std::cout << "    调试:   " << std::boolalpha
              << tree.get<bool>("app.debug", false) << "\n";
    std::cout << "    DB主机: " << tree.get<std::string>("database.host") << "\n";
    std::cout << "    DB端口: " << tree.get<int>("database.port") << "\n";
    std::cout << "    日志级别: " << tree.get<std::string>("log.level") << "\n";
    std::cout << "    最大连接数: "
              << tree.get<int>("database.max_connections") << "\n";

    // 修改配置并写回
    tree.put("app.version", "2.1.1");
    tree.put("log.level", "info");
    tree.put("log.auto_flush", true);  // 新增配置项

    pt::write_ini("config_modified.ini", tree);
    std::cout << "\n  修改后的配置已写入 config_modified.ini\n";
    std::cout << "  （版本号升级为 2.1.1，日志级别改为 info，新增 auto_flush）\n";

    std::cout << "\n  INI 格式特点：\n";
    std::cout << "    - 用 [section] 分组\n";
    std::cout << "    - key = value 格式\n";
    std::cout << "    - 用 ; 或 # 写注释\n";
    std::cout << "    - 只能有两层（section + key），不支持深层嵌套\n";
}

// ============================================================================
// 3. JSON 文件读写
// ============================================================================
// JSON 支持多层嵌套和数组，适合复杂配置。
// 注意：Boost.PropertyTree 的 JSON 支持是有限的，不是完整的 JSON 解析器。
// （比如所有值都存为字符串，不区分数字/布尔/字符串类型）

void demo_json_file() {
    std::cout << "\n===== 3. JSON 配置文件 =====\n";

    // 构建一个 JSON 结构的 ptree
    pt::ptree tree;

    // 简单属性
    tree.put("name", "DemoApp");
    tree.put("version", "1.0.0");
    tree.put("port", 8080);

    // 嵌套对象（database 是一个子树）
    pt::ptree db_node;
    db_node.put("host", "localhost");
    db_node.put("port", 27017);
    db_node.put("name", "mydb");
    tree.put_child("database", db_node);

    // 数组（用 push_back 添加空 key 的子节点）
    pt::ptree servers_node;
    for (int i = 0; i < 3; i++) {
        pt::ptree server;
        server.put("host", "server" + std::to_string(i) + ".example.com");
        server.put("port", 9000 + i);
        server.put("weight", (i + 1) * 10);
        servers_node.push_back(std::make_pair("", server));
    }
    tree.put_child("servers", servers_node);

    // 字符串数组
    pt::ptree features_node;
    for (const auto& f : {"login", "payment", "push", "analytics"}) {
        pt::ptree item;
        item.put_value(f);
        features_node.push_back(std::make_pair("", item));
    }
    tree.put_child("features", features_node);

    // 写入 JSON 文件
    pt::write_json("config.json", tree);
    std::cout << "  已写入 config.json\n\n";

    // 读取 JSON 文件
    pt::ptree read_tree;
    pt::read_json("config.json", read_tree);

    std::cout << "  读取 JSON：\n";
    std::cout << "    应用名: " << read_tree.get<std::string>("name") << "\n";
    std::cout << "    端口: " << read_tree.get<int>("port") << "\n";
    std::cout << "    数据库: " << read_tree.get<std::string>("database.host")
              << ":" << read_tree.get<int>("database.port") << "\n";

    // 遍历数组
    std::cout << "    服务器列表:\n";
    for (auto& item : read_tree.get_child("servers")) {
        // item.first 是 key（数组项为空字符串）
        // item.second 是值（子树）
        pt::ptree& server = item.second;
        std::cout << "      - " << server.get<std::string>("host")
                  << ":" << server.get<int>("port")
                  << " (权重: " << server.get<int>("weight") << ")\n";
    }

    // 遍历字符串数组
    std::cout << "    功能列表: ";
    for (auto& item : read_tree.get_child("features")) {
        std::cout << item.second.data() << " ";
    }
    std::cout << "\n";

    std::cout << "\n  JSON 格式特点：\n";
    std::cout << "    - 支持深层嵌套\n";
    std::cout << "    - 支持数组\n";
    std::cout << "    - 注意：PropertyTree 的 JSON 解析把所有值当字符串存\n";
    std::cout << "    - 适合配置，不适合复杂 JSON 数据处理\n";
    std::cout << "    - 如果要处理复杂 JSON，推荐用 RapidJSON 或 nlohmann/json\n";
}

// ============================================================================
// 4. XML 文件读写
// ============================================================================
void demo_xml_file() {
    std::cout << "\n===== 4. XML 配置文件 =====\n";

    // 构建 XML 结构
    pt::ptree tree;

    // 根元素的属性（用 <xmlattr> 子树）
    pt::ptree& root = tree.put("config", "");
    root.put("<xmlattr>.version", "1.0");
    root.put("<xmlattr>.encoding", "UTF-8");

    // 数据库配置
    pt::ptree db;
    db.put("<xmlattr>.type", "mysql");
    db.put("host", "localhost");
    db.put("port", 3306);
    db.put("user", "root");
    db.put("password", "123456");
    root.add_child("database", db);

    // 多个 module
    pt::ptree modules;
    for (int i = 1; i <= 3; i++) {
        pt::ptree mod;
        mod.put("<xmlattr>.name", "module_" + std::to_string(i));
        mod.put("<xmlattr>.enabled", i % 2 == 0 ? "true" : "false");
        mod.put("path", "/opt/modules/module" + std::to_string(i));
        modules.add_child("module", mod);
    }
    root.add_child("modules", modules);

    // 写入 XML
    pt::write_xml("config.xml", tree, std::locale(),
                  pt::xml_writer_make_settings<std::string>(' ', 2));
    std::cout << "  已写入 config.xml\n\n";

    // 读取 XML
    pt::ptree read_tree;
    pt::read_xml("config.xml", read_tree);

    std::cout << "  读取 XML：\n";
    std::cout << "    配置版本: "
              << read_tree.get<std::string>("config.<xmlattr>.version") << "\n";
    std::cout << "    数据库类型: "
              << read_tree.get<std::string>("config.database.<xmlattr>.type") << "\n";
    std::cout << "    数据库地址: "
              << read_tree.get<std::string>("config.database.host") << "\n";

    // 遍历 modules
    std::cout << "    模块列表:\n";
    for (auto& item : read_tree.get_child("config.modules")) {
        if (item.first == "module") {
            pt::ptree& mod = item.second;
            std::cout << "      - " << mod.get<std::string>("<xmlattr>.name")
                      << " (enabled: " << mod.get<std::string>("<xmlattr>.enabled")
                      << ")\n";
        }
    }

    std::cout << "\n  XML 格式特点：\n";
    std::cout << "    - 支持属性（xmlattr）\n";
    std::cout << "    - 支持注释、CDATA 等\n";
    std::cout << "    - 相对较重，配置文件用得越来越少\n";
    std::cout << "    - Java 生态用得较多，C++ 项目一般用 INI 或 JSON\n";
}

// ============================================================================
// 4.5 INFO 格式（Boost.PropertyTree 自带的简洁格式）
// ============================================================================
void demo_info_file() {
    std::cout << "\n===== 4.5 INFO 配置文件 =====\n";

    pt::ptree tree;
    tree.put("server.host", "0.0.0.0");
    tree.put("server.port", 8080);
    tree.put("server.workers", 4);
    tree.put("logging.level", "info");

    pt::write_info("config.info", tree);

    std::cout << "  已写入 config.info，内容如下：\n\n";
    std::ifstream ifs("config.info");
    std::cout << ifs.rdbuf() << "\n";

    pt::ptree loaded;
    pt::read_info("config.info", loaded);
    std::cout << "  读回 server.port = " << loaded.get<int>("server.port") << "\n";
    std::cout << "  读回 logging.level = "
              << loaded.get<std::string>("logging.level") << "\n";

    std::cout << "\n  INFO 格式特点：\n";
    std::cout << "    - Boost 自有格式，用 {} 表示层级，没有引号和逗号\n";
    std::cout << "    - 适合纯 C++ 项目内部使用，其他语言生态支持较少\n";
}

// ============================================================================
// 5. 遍历操作
// ============================================================================
void demo_traversal() {
    std::cout << "\n===== 5. 遍历与查找 =====\n";

    pt::ptree tree;
    tree.put("app.name", "TestApp");
    tree.put("app.version", "1.0");
    tree.put("app.debug", "true");

    tree.put("db.host", "localhost");
    tree.put("db.port", "3306");
    tree.put("db.user", "root");

    tree.put("log.level", "info");
    tree.put("log.file", "app.log");

    // 遍历一级节点
    std::cout << "  一级节点:\n";
    for (auto& item : tree) {
        std::cout << "    " << item.first
                  << " = " << item.second.data() << "\n";
    }

    // 递归遍历所有节点（辅助函数）
    std::function<void(const pt::ptree&, const std::string&)> print_tree =
        [&](const pt::ptree& node, const std::string& path) {
            for (auto& item : node) {
                std::string new_path = path.empty()
                    ? item.first
                    : path + "." + item.first;
                if (!item.second.data().empty()) {
                    std::cout << "    " << new_path
                              << " = " << item.second.data() << "\n";
                }
                print_tree(item.second, new_path);
            }
        };

    std::cout << "\n  所有节点（递归）:\n";
    print_tree(tree, "");

    // count：统计子节点数量
    std::cout << "\n  db 下的子节点数: "
              << tree.get_child("db").size() << "\n";

    // find：查找子节点（只找直接子节点）
    auto it = tree.find("app");
    if (it != tree.not_found()) {
        std::cout << "  找到了 app 节点\n";
    }

    std::cout << "\n  遍历的三种方式：\n";
    std::cout << "    - begin()/end() 遍历直接子节点\n";
    std::cout << "    - 递归遍历所有后代节点\n";
    std::cout << "    - find() 查找指定子节点\n";
}

// ============================================================================
// 6. 异常处理与健壮性
// ============================================================================
void demo_error_handling() {
    std::cout << "\n===== 6. 异常处理 =====\n";

    pt::ptree tree;
    tree.put("server.port", 8080);

    // 方式1：get 带默认值
    // 注意：默认值只在“路径不存在”时生效；如果键存在但值转换失败，
    // 仍然会抛 ptree_bad_data，默认值救不了你。
    int timeout = tree.get<int>("server.timeout", 30);
    std::cout << "  带默认值: timeout = " << timeout << "\n";

    // 方式2：get_optional（灵活）
    auto host = tree.get_optional<std::string>("server.host");
    if (host) {
        std::cout << "  host = " << *host << "\n";
    } else {
        std::cout << "  host 未配置\n";
    }

    // 方式3：try-catch（不推荐频繁使用）
    try {
        std::string user = tree.get<std::string>("server.user");
        std::cout << "  user = " << user << "\n";
    } catch (const pt::ptree_bad_path& e) {
        std::cout << "  捕获异常：路径不存在 - " << e.what() << "\n";
    } catch (const pt::ptree_bad_data& e) {
        std::cout << "  捕获异常：类型转换失败 - " << e.what() << "\n";
    }

    // 类型转换失败：即使提供了默认值，也不能吞掉“值存在但非法”的错误
    tree.put("server.port", "not_a_number");
    try {
        int port = tree.get<int>("server.port", 8080);
        std::cout << "  port = " << port << "\n";
    } catch (const pt::ptree_bad_data& e) {
        std::cout << "  类型转换失败（预期，默认值不会生效）: "
                  << e.what() << "\n";
    }

    std::cout << "\n  最佳实践：\n";
    std::cout << "    - 必须存在的配置用 get()，缺失就是错误\n";
    std::cout << "    - get(默认值) 只能处理“键缺失”，不能处理“值类型错误”\n";
    std::cout << "    - 可选配置、或外部输入不可信时用 get_optional()/显式校验\n";
    std::cout << "    - 边界处（读文件、收网络请求）要 try-catch 或校验类型\n";
}

// ============================================================================
// 7. 实战：一个完整的应用配置类
// ============================================================================
// 实际项目中，一般会封装一个 Config 类，集中管理所有配置。

class AppConfig {
public:
    // 加载配置
    bool load(const std::string& file_path) {
        try {
            pt::read_ini(file_path, tree_);
        } catch (const std::exception& e) {
            std::cerr << "加载配置失败: " << e.what() << "\n";
            return false;
        }
        return validate();
    }

    // 保存配置
    bool save(const std::string& file_path) const {
        try {
            pt::write_ini(file_path, tree_);
        } catch (const std::exception& e) {
            std::cerr << "保存配置失败: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    // 校验配置完整性
    bool validate() const {
        bool ok = true;

        // 必填项检查
        std::vector<std::string> required = {
            "app.name",
            "database.host",
            "database.port",
            "database.user"
        };

        for (const auto& key : required) {
            if (!tree_.get_optional<std::string>(key)) {
                std::cerr << "配置缺失: " << key << "\n";
                ok = false;
            }
        }

        // 范围检查
        // get_optional<int> 在“键缺失”和“值无法转换”两种情况下都返回空，
        // 比 get<int>(默认值) 更适合校验外部配置文件。
        auto port = tree_.get_optional<int>("database.port");
        if (!port) {
            std::cerr << "端口号缺失或不是整数\n";
            ok = false;
        } else if (*port <= 0 || *port > 65535) {
            std::cerr << "端口号不合法: " << *port << "\n";
            ok = false;
        }

        // 可选字段存在时才校验；存在但不是整数同样算配置错误
        if (tree_.get_child_optional("database.max_connections")) {
            auto max_conn = tree_.get_optional<int>("database.max_connections");
            if (!max_conn || *max_conn <= 0) {
                std::cerr << "max_connections 必须是正整数\n";
                ok = false;
            }
        }

        return ok;
    }

    // 便捷的访问方法
    std::string app_name() const {
        return tree_.get<std::string>("app.name");
    }
    std::string db_host() const {
        return tree_.get<std::string>("database.host");
    }
    int db_port() const {
        return tree_.get<int>("database.port");
    }
    std::string log_level() const {
        return tree_.get<std::string>("log.level", "info");
    }
    int max_connections() const {
        return tree_.get<int>("database.max_connections", 10);
    }

    // 原始 ptree 访问（高级用法）
    const pt::ptree& tree() const { return tree_; }

private:
    pt::ptree tree_;
};

void demo_app_config() {
    std::cout << "\n===== 7. 实战：完整的应用配置类 =====\n";

    // 生成测试配置文件
    std::ofstream("app_config.ini") << R"(
[app]
name = OrderService
version = 3.2.1

[database]
host = db.example.com
port = 5432
user = orderservice
password = encrypted_password_here
max_connections = 50

[log]
level = info
file = /var/log/order_service.log
max_size_mb = 200
)";

    // 使用配置类
    AppConfig config;
    if (config.load("app_config.ini")) {
        std::cout << "  配置加载成功！\n\n";
        std::cout << "    应用名: " << config.app_name() << "\n";
        std::cout << "    数据库: " << config.db_host()
                  << ":" << config.db_port() << "\n";
        std::cout << "    日志级别: " << config.log_level() << "\n";
        std::cout << "    最大连接数: " << config.max_connections() << "\n";
    } else {
        std::cout << "  配置加载失败！\n";
    }

    std::cout << "\n  封装配置类的好处：\n";
    std::cout << "    - 集中管理，调用方不用关心配置文件格式\n";
    std::cout << "    - 可以做校验，提前发现配置错误\n";
    std::cout << "    - 可以加缓存，避免反复解析文件\n";
    std::cout << "    - 将来换配置格式（如从INI改JSON），只改一个类\n";
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
    std::cout << "  配置管理是软件工程里的一件大事。\n";
    std::cout << "  我见过太多人把 IP、端口、超时时间全硬编码在代码里，\n";
    std::cout << "  换个环境就要重新编译，蠢得很。\n";
    std::cout << "\n";
    std::cout << "  记住：凡是可能变的东西，都应该放到配置文件里。\n";
    std::cout << "  代码是骨架，配置是血肉。骨架要稳，血肉要活。\n";
    std::cout << "\n";
    std::cout << "  PropertyTree 这个库，小而美，中小项目完全够用。\n";
    std::cout << "  INI 格式简单，人类也能读能改，首选。\n";
    std::cout << "  需要嵌套和数组就用 JSON。\n";
    std::cout << "  XML 除非项目要求，否则尽量别自找麻烦。\n";
    std::cout << "\n";
    std::cout << "  几个经验之谈：\n";
    std::cout << "  1️⃣ 配置加载时一定要校验，缺了关键配置直接报错退出\n";
    std::cout << "  2️⃣  密码、密钥等敏感信息不要明文存配置文件\n";
    std::cout << "  3️⃣  配置项要有合理的默认值\n";
    std::cout << "  4️⃣  大项目用配置中心（比如 Apollo、Nacos），原理是通的\n";
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
    std::cout << "  1. 给你的学生管理系统加上配置文件支持：\n";
    std::cout << "     - 数据文件路径\n";
    std::cout << "     - 最大学生数\n";
    std::cout << "     - 日志级别\n";
    std::cout << "     - 用 INI 格式，写一个 Config 类封装\n";
    std::cout << "\n";
    std::cout << "  2. 比较四种配置格式的优缺点：\n";
    std::cout << "     INI / JSON / XML / INFO，各适合什么场景？\n";
    std::cout << "     写一段你自己的理解。\n";
    std::cout << "\n";
    std::cout << "  3. 思考：配置热更新怎么做？\n";
    std::cout << "     （提示：定时重读文件 + 信号通知 + 版本号）\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  Boost.PropertyTree 配置管理深度解析\n";
    std::cout << "  项目必备技能 —— 告别硬编码，拥抱配置化\n";
    std::cout << "============================================================\n";

    demo_basic_ptree();
    demo_ini_file();
    demo_json_file();
    demo_xml_file();
    demo_info_file();
    demo_traversal();
    demo_error_handling();
    demo_app_config();

    laoshifu_says();
    exercises();

    return 0;
}
