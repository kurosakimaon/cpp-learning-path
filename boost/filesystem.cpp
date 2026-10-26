/**
 * Boost.Filesystem 文件系统操作 Demo
 *
 * 知识点覆盖：
 *   1. path 类：路径拼接、分解、规范化、比较、跨平台处理
 *   2. 文件属性：大小、权限、时间戳、文件类型判断
 *   3. 目录迭代：directory_iterator / recursive_directory_iterator
 *   4. 符号链接（symlink）：创建、读取、检测
 *   5. 文件操作：复制、移动、删除、重命名、创建目录
 *   6. 错误码处理：error_code vs 异常，两种模式对比
 *   7. 文件流：ifstream/ofstream 与 path 的集成
 *   8. Boost.Filesystem vs C++17 std::filesystem 的异同
 *
 * Boost 优势：
 *   - Boost.Filesystem 是 C++17 std::filesystem 的原型和参考实现
 *   - 在 C++17 之前的环境中提供完整的文件系统操作
 *   - 提供一些标准库没有的功能（如 unique_path、更丰富的路径操作）
 *   - 与 Boost 其他库（如 Boost.Iostreams、Boost.Serialization）良好集成
 *   - 更成熟稳定，在旧编译器和旧平台上兼容性更好
 *
 * 编译命令（示例）：
 *   Linux:        g++ -std=c++17 filesystem.cpp -o filesystem -lboost_filesystem
 *   MinGW/MSYS2:  g++ -std=c++17 filesystem.cpp -o filesystem -lboost_filesystem-mt
 *   （Windows 预编译库常带 -mt 后缀，以实际库文件名为准）
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <iomanip>

// ========== Boost 头文件 ==========
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>

namespace fs = boost::filesystem;
using boost::system::error_code;
using boost::system::system_error;

// ============================================================
// 一、path 类深入用法
// ============================================================
// path 是 Boost.Filesystem 的核心类，代表一个文件系统路径。
// 设计要点：
//   - 内部存储使用平台原生的字符编码（Windows 为 wchar_t，Linux 为 char）
//   - 提供通用的 POSIX 风格路径语法，自动转换到平台格式
//   - 支持迭代器、比较、拼接、分解等操作
//   - 重载了 operator/= 和 operator/ 用于路径拼接（比字符串拼接更安全）

void path_demo() {
    std::cout << "\n--- 1. path 类深入用法 ---" << std::endl;

    // 1.1 路径构造
    std::cout << "1.1 路径构造与表示:" << std::endl;
    fs::path p1 = "C:\\Users\\test\\data.txt";  // Windows 风格
    fs::path p2 = "/home/user/data.txt";         // POSIX 风格
    fs::path p3("folder/subfolder/file");       // 相对路径

    std::cout << "  原生格式: " << p1.string() << std::endl;
    std::cout << "  通用格式: " << p1.generic_string() << std::endl;
    std::cout << "  父路径: " << p1.parent_path() << std::endl;
    std::cout << "  文件名: " << p1.filename() << std::endl;
    std::cout << "  扩展名: " << p1.extension() << std::endl;
    std::cout << "  主干名: " << p1.stem() << std::endl;
    std::cout << "  根路径: " << p1.root_path() << std::endl;
    std::cout << "  根目录: " << p1.root_directory() << std::endl;
    std::cout << "  根名: " << p1.root_name() << std::endl;

    // 1.2 路径拼接（推荐使用 / 运算符，自动处理分隔符）
    std::cout << "\n1.2 路径拼接:" << std::endl;
    fs::path base = "C:\\project";
    fs::path full_path = base / "src" / "main.cpp";
    std::cout << "  拼接结果: " << full_path << std::endl;

    // 1.3 路径规范化（解析 . 和 ..）
    std::cout << "\n1.3 路径规范化:" << std::endl;
    fs::path messy_path = "a/./b/../b/c/../d";
    std::cout << "  原始路径: " << messy_path << std::endl;
    std::cout << "  规范化: " << fs::weakly_canonical(messy_path) << std::endl;
    // canonical() 需要文件实际存在，weakly_canonical() 不要求

    // 1.4 路径比较
    std::cout << "\n1.4 路径比较:" << std::endl;
    fs::path pa = "path/to/file";
    fs::path pb = "path/to/other";
    std::cout << "  pa == pb: " << std::boolalpha << (pa == pb) << std::endl;
    std::cout << "  pa < pb: " << std::boolalpha << (pa < pb) << std::endl;
    // 注意：路径比较是字典序的，不考虑文件系统中的实际等价性

    // 1.5 路径分解迭代器
    std::cout << "\n1.5 路径迭代器:" << std::endl;
    std::cout << "  路径组成部分: ";
    for (const auto& part : full_path) {
        std::cout << part << " / ";
    }
    std::cout << std::endl;

    // 1.6 路径替换操作
    std::cout << "\n1.6 路径替换:" << std::endl;
    fs::path file = "document.txt";
    std::cout << "  原始: " << file << std::endl;
    file.replace_extension(".md");
    std::cout << "  替换扩展名: " << file << std::endl;
    file.replace_filename("readme.md");
    std::cout << "  替换文件名: " << file << std::endl;

    // 1.7 路径查询
    std::cout << "\n1.7 路径属性查询:" << std::endl;
    std::cout << "  is_absolute: " << p1.is_absolute() << std::endl;
    std::cout << "  is_relative: " << p3.is_relative() << std::endl;
    std::cout << "  has_filename: " << p1.has_filename() << std::endl;
    std::cout << "  has_parent_path: " << p1.has_parent_path() << std::endl;
    std::cout << "  has_extension: " << p1.has_extension() << std::endl;
    std::cout << "  empty: " << fs::path().empty() << std::endl;
}

// ============================================================
// 二、文件属性操作
// ============================================================
void file_status_demo(const fs::path& test_dir) {
    std::cout << "\n--- 2. 文件属性 ---" << std::endl;

    // 创建测试文件
    fs::path test_file = test_dir / "example.txt";
    std::ofstream(test_file.string()) << "Hello, Boost.Filesystem!";

    // 2.1 文件类型判断
    std::cout << "2.1 文件类型判断:" << std::endl;
    std::cout << "  exists: " << fs::exists(test_file) << std::endl;
    std::cout << "  is_regular_file: " << fs::is_regular_file(test_file) << std::endl;
    std::cout << "  is_directory: " << fs::is_directory(test_file) << std::endl;
    std::cout << "  is_directory(test_dir): " << fs::is_directory(test_dir) << std::endl;
    std::cout << "  is_empty(test_dir): " << fs::is_empty(test_dir) << std::endl;

    // 2.2 文件大小
    std::cout << "\n2.2 文件大小:" << std::endl;
    if (fs::is_regular_file(test_file)) {
        std::cout << "  文件大小: " << fs::file_size(test_file) << " 字节" << std::endl;
    }

    // 2.3 文件时间戳
    std::cout << "\n2.3 文件时间戳:" << std::endl;
    auto ftime = fs::last_write_time(test_file);
    // Boost.Filesystem 的时间是 std::time_t 格式（与 std::filesystem 不同）
    std::time_t t = ftime;
    std::cout << "  最后修改时间: " << std::ctime(&t);

    // 2.4 文件权限（perms 是位掩码）
    std::cout << "\n2.4 文件权限:" << std::endl;
    fs::perms p = fs::status(test_file).permissions();
    std::cout << "  权限位: " << std::oct
              << static_cast<unsigned>(p) << std::dec << std::endl;
    std::cout << "  所有者可读: "
              << ((p & fs::owner_read) != fs::perms::no_perms) << std::endl;
    std::cout << "  所有者可写: "
              << ((p & fs::owner_write) != fs::perms::no_perms) << std::endl;

    // 2.5 file_status vs symlink_status
    std::cout << "\n2.5 file_status vs symlink_status:" << std::endl;
    std::cout << "  status(): 跟随符号链接，返回目标文件的状态" << std::endl;
    std::cout << "  symlink_status(): 不跟随符号链接，返回链接本身的状态" << std::endl;

    // 修改权限示例（仅演示，Windows 下可能不支持全部权限位）
    // fs::permissions(test_file, fs::owner_read | fs::owner_write);
}

// ============================================================
// 三、目录迭代
// ============================================================
void directory_iteration_demo(const fs::path& test_dir) {
    std::cout << "\n--- 3. 目录迭代 ---" << std::endl;

    // 创建测试目录结构
    fs::create_directories(test_dir / "subdir1");
    fs::create_directories(test_dir / "subdir2");
    std::ofstream((test_dir / "file1.txt").string()) << "file1";
    std::ofstream((test_dir / "file2.dat").string()) << "file2";
    std::ofstream((test_dir / "subdir1/nested.txt").string()) << "nested";

    // 3.1 directory_iterator：单层目录迭代
    std::cout << "3.1 directory_iterator（单层）:" << std::endl;
    try {
        for (fs::directory_iterator it(test_dir); it != fs::directory_iterator(); ++it) {
            const auto& entry = *it;
            std::string type = fs::is_directory(entry.path()) ? "[DIR]" : "[FILE]";
            std::cout << "  " << type << " " << entry.path().filename() << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "  迭代错误: " << e.what() << std::endl;
    }

    // 3.2 recursive_directory_iterator：递归目录迭代
    std::cout << "\n3.2 recursive_directory_iterator（递归）:" << std::endl;
    int file_count = 0;
    for (fs::recursive_directory_iterator it(test_dir);
         it != fs::recursive_directory_iterator(); ++it) {
        if (fs::is_regular_file(it->path())) {
            ++file_count;
            // 注意：relative_path() 只是去掉根名/根目录，不会去掉 test_dir。
            // 要显示“相对于 test_dir”的路径，用 lexically_relative()。
            std::cout << "  文件: " << it->path().lexically_relative(test_dir)
                      << " (" << fs::file_size(it->path()) << " bytes)" << std::endl;
        }
    }
    std::cout << "  总文件数: " << file_count << std::endl;

    // 3.3 迭代控制：depth()、pop()、no_push()
    std::cout << "\n3.3 递归迭代控制:" << std::endl;
    fs::recursive_directory_iterator it(test_dir);
    for (; it != fs::recursive_directory_iterator(); ++it) {
        std::cout << "  深度 " << it.depth() << ": "
                  << it->path().filename() << std::endl;
        // 示例：进入 subdir1 后不继续深入
        // if (it->path().filename() == "subdir1") {
        //     it.disable_recursion_pending();
        // }
    }

    // 3.4 按扩展名筛选（实战技巧）
    std::cout << "\n3.4 按扩展名筛选 .txt 文件:" << std::endl;
    for (fs::recursive_directory_iterator it(test_dir);
         it != fs::recursive_directory_iterator(); ++it) {
        if (it->path().extension() == ".txt") {
            std::cout << "  " << it->path() << std::endl;
        }
    }
}

// ============================================================
// 四、符号链接
// ============================================================
void symlink_demo(const fs::path& test_dir) {
    std::cout << "\n--- 4. 符号链接 ---" << std::endl;

    // 创建测试文件
    fs::path target = test_dir / "original.txt";
    std::ofstream(target.string()) << "original content";

    // 4.1 创建符号链接
    std::cout << "4.1 创建符号链接:" << std::endl;
    fs::path link = test_dir / "link_to_original";
    try {
        fs::create_symlink(target, link);
        std::cout << "  创建符号链接: " << link << " -> " << target << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cout << "  创建失败 (Windows 可能需要管理员权限): " << e.code().message() << std::endl;
        std::cout << "  提示：Windows 下创建符号链接需要开发者模式或管理员权限" << std::endl;
        return;
    }

    // 4.2 读取符号链接目标
    std::cout << "\n4.2 读取符号链接:" << std::endl;
    if (fs::is_symlink(link)) {
        fs::path link_target = fs::read_symlink(link);
        std::cout << "  链接指向: " << link_target << std::endl;
    }

    // 4.3 status vs symlink_status 的区别
    std::cout << "\n4.3 status vs symlink_status:" << std::endl;
    std::cout << "  is_symlink(link): " << fs::is_symlink(link) << std::endl;
    std::cout << "  is_regular_file(link) [跟随链接]: "
              << fs::is_regular_file(link) << std::endl;
    // symlink_status 用于检查符号链接本身的状态
    fs::file_status st = fs::symlink_status(link);
    std::cout << "  symlink_status type == symlink_file: "
              << (st.type() == fs::symlink_file) << std::endl;

    // 4.4 目录符号链接
    std::cout << "\n4.4 目录符号链接:" << std::endl;
    fs::path dir_target = test_dir / "subdir1";
    fs::path dir_link = test_dir / "dir_link";
    try {
        fs::create_directory_symlink(dir_target, dir_link);
        std::cout << "  目录符号链接创建成功" << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cout << "  创建目录符号链接失败: " << e.code().message() << std::endl;
    }
}

// ============================================================
// 五、文件操作（复制、移动、删除、重命名）
// ============================================================
void file_operations_demo(const fs::path& test_dir) {
    std::cout << "\n--- 5. 文件操作 ---" << std::endl;

    // 创建源文件
    fs::path src = test_dir / "source.txt";
    std::ofstream(src.string()) << "source content for testing";

    // 5.1 文件复制
    std::cout << "5.1 文件复制:" << std::endl;
    fs::path dst = test_dir / "dest.txt";
    fs::copy_file(src, dst);
    std::cout << "  复制: " << src.filename() << " -> " << dst.filename() << std::endl;
    std::cout << "  目标存在: " << fs::exists(dst) << std::endl;

    // copy_options::overwrite_existing：覆盖已存在的文件
    // copy_options::skip_existing：跳过已存在的文件（目标保持原样）
    std::ofstream(src.string(), std::ios::app) << " updated";
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
    std::cout << "  覆盖复制完成" << std::endl;

    // 5.2 目录复制
    std::cout << "\n5.2 目录复制:" << std::endl;
    fs::path src_dir = test_dir / "subdir1";
    fs::path dst_dir = test_dir / "subdir1_copy";
    fs::copy(src_dir, dst_dir);  // copy 函数会自动处理文件/目录
    std::cout << "  目录复制: " << src_dir.filename() << " -> " << dst_dir.filename() << std::endl;
    std::cout << "  目标目录存在: " << fs::exists(dst_dir) << std::endl;

    // 5.3 文件移动/重命名
    std::cout << "\n5.3 文件移动 / 重命名:" << std::endl;
    fs::path moved = test_dir / "moved.txt";
    fs::rename(dst, moved);
    std::cout << "  重命名 dest.txt -> moved.txt" << std::endl;
    std::cout << "  dest.txt 存在: " << fs::exists(dst) << std::endl;
    std::cout << "  moved.txt 存在: " << fs::exists(moved) << std::endl;

    // 5.4 文件删除
    std::cout << "\n5.4 文件删除:" << std::endl;
    bool removed = fs::remove(moved);
    std::cout << "  删除 moved.txt: " << (removed ? "成功" : "失败") << std::endl;

    // 递归删除目录
    fs::path to_remove = test_dir / "subdir1_copy";
    uintmax_t count = fs::remove_all(to_remove);
    std::cout << "  递归删除 subdir1_copy，共删除 " << count << " 个条目" << std::endl;

    // 5.5 创建目录
    std::cout << "\n5.5 创建目录:" << std::endl;
    fs::path new_dir = test_dir / "a" / "b" / "c";
    bool created = fs::create_directories(new_dir);  // 递归创建
    std::cout << "  创建嵌套目录 a/b/c: " << (created ? "成功" : "已存在") << std::endl;
}

// ============================================================
// 六、错误处理：error_code vs 异常
// ============================================================
void error_handling_demo(const fs::path& test_dir) {
    std::cout << "\n--- 6. 错误处理方式 ---" << std::endl;

    // 6.1 异常方式（代码简洁，但需要 try-catch）
    std::cout << "6.1 异常方式:" << std::endl;
    try {
        fs::file_size(test_dir / "nonexistent.txt");
    } catch (const fs::filesystem_error& e) {
        std::cout << "  捕获 filesystem_error:" << std::endl;
        std::cout << "    what(): " << e.what() << std::endl;
        std::cout << "    path1(): " << e.path1() << std::endl;
        std::cout << "    code().value(): " << e.code().value() << std::endl;
        std::cout << "    code().message(): " << e.code().message() << std::endl;
    }

    // 6.2 error_code 方式（更适合批量操作，不中断流程）
    std::cout << "\n6.2 error_code 方式:" << std::endl;
    error_code ec;
    uintmax_t size = fs::file_size(test_dir / "nonexistent.txt", ec);
    if (ec) {
        std::cout << "  获取文件大小失败: " << ec.message() << std::endl;
        std::cout << "  错误码值: " << ec.value() << std::endl;
        std::cout << "  错误类别: " << ec.category().name() << std::endl;
        // 可以比较特定的错误条件
        if (ec == boost::system::errc::no_such_file_or_directory) {
            std::cout << "  确认：文件不存在" << std::endl;
        }
    } else {
        std::cout << "  文件大小: " << size << std::endl;
    }

    // 6.3 推荐场景
    std::cout << "\n6.3 使用建议:" << std::endl;
    std::cout << "  异常方式：适合简单脚本，出错即终止" << std::endl;
    std::cout << "  error_code：适合生产代码，精细控制错误处理" << std::endl;
    std::cout << "  批量操作（如目录遍历）中推荐 error_code" << std::endl;
}

// ============================================================
// 七、空间查询与其他实用功能
// ============================================================
void misc_demo(const fs::path& test_dir) {
    std::cout << "\n--- 7. 其他实用功能 ---" << std::endl;

    // 7.1 磁盘空间查询
    std::cout << "7.1 磁盘空间:" << std::endl;
    fs::space_info info = fs::space(test_dir);
    std::cout << "  总容量: " << info.capacity / (1024 * 1024 * 1024) << " GB" << std::endl;
    std::cout << "  可用空间: " << info.free / (1024 * 1024 * 1024) << " GB" << std::endl;
    std::cout << "  空闲空间(普通用户): " << info.available / (1024 * 1024 * 1024) << " GB" << std::endl;

    // 7.2 当前工作目录
    std::cout << "\n7.2 工作目录:" << std::endl;
    std::cout << "  当前目录: " << fs::current_path() << std::endl;

    // 7.3 临时目录
    std::cout << "\n7.3 临时目录:" << std::endl;
    std::cout << "  系统临时目录: " << fs::temp_directory_path() << std::endl;

    // 7.4 生成唯一文件名（Boost 特有，标准库无此功能）
    std::cout << "\n7.4 unique_path（生成唯一路径，Boost 特有）:" << std::endl;
    fs::path unique = fs::unique_path("temp-%%%%-%%%%-%%%%-%%%%.tmp");
    std::cout << "  生成的唯一文件名: " << unique << std::endl;
    // % 会被替换为随机十六进制字符，适合创建临时文件

    // 7.5 路径比较与等价性
    std::cout << "\n7.5 路径比较与等价性:" << std::endl;
    fs::path p1 = test_dir / "source.txt";
    fs::path p2 = test_dir / "." / "source.txt";
    std::cout << "  p1 = " << p1.filename() << std::endl;
    std::cout << "  p2 = " << p2.filename() << std::endl;
    std::cout << "  路径字符串相等: " << (p1 == p2) << std::endl;
    // 注意：路径字符串相等不代表文件系统中是同一个文件
    // 要判断是否指向同一文件，需要使用 equivalent()
    if (fs::exists(p1) && fs::exists(p2)) {
        std::cout << "  equivalent（同一文件）: " << fs::equivalent(p1, p2) << std::endl;
    }
}

// ============================================================
// 八、Boost.Filesystem vs std::filesystem 对比
// ============================================================
void comparison_summary() {
    std::cout << "\n--- 8. Boost.Filesystem vs C++17 std::filesystem ---" << std::endl;
    std::cout << "  相同点：" << std::endl;
    std::cout << "    - 核心 API 基本一致（path、directory_iterator 等）" << std::endl;
    std::cout << "    - 错误处理模式一致（error_code + 异常）" << std::endl;
    std::cout << "    - 设计理念相同（基于 POSIX 标准）" << std::endl;
    std::cout << std::endl;
    std::cout << "  主要差异：" << std::endl;
    std::cout << "    1. 命名空间：boost::filesystem vs std::filesystem" << std::endl;
    std::cout << "    2. 时间类型：" << std::endl;
    std::cout << "       - Boost: std::time_t (兼容旧代码)" << std::endl;
    std::cout << "       - std: std::filesystem::file_time_type (基于 chrono)" << std::endl;
    std::cout << "    3. Boost 独有功能：" << std::endl;
    std::cout << "       - unique_path()：生成唯一文件名" << std::endl;
    std::cout << "       - 更丰富的字符串编码转换支持" << std::endl;
    std::cout << "       - 与 Boost.Iostreams 集成" << std::endl;
    std::cout << "    4. std 新增功能：" << std::endl;
    std::cout << "       - std::filesystem::path::u8string 等" << std::endl;
    std::cout << "       - 更完善的权限枚举" << std::endl;
    std::cout << std::endl;
    std::cout << "  选择建议：" << std::endl;
    std::cout << "    - C++17 及以上：优先使用 std::filesystem（标准库）" << std::endl;
    std::cout << "    - C++14 及以下：必须使用 Boost.Filesystem" << std::endl;
    std::cout << "    - 需要 unique_path 等 Boost 特有功能：用 Boost" << std::endl;
    std::cout << "    - 已有 Boost 依赖的项目：用 Boost 更统一" << std::endl;
}

// ============================================================
// Demo 入口函数
// ============================================================
int main() {
    std::cout << "========== Boost.Filesystem 文件系统操作 Demo ==========" << std::endl;

    // 创建测试目录（使用系统临时目录）
    fs::path test_dir = fs::temp_directory_path() / "boost_fs_demo";
    fs::remove_all(test_dir);  // 清理旧数据
    fs::create_directories(test_dir);
    std::cout << "测试目录: " << test_dir << std::endl;

    try {
        // 1. path 类
        path_demo();

        // 2. 文件属性
        file_status_demo(test_dir);

        // 3. 目录迭代
        directory_iteration_demo(test_dir);

        // 4. 符号链接
        symlink_demo(test_dir);

        // 5. 文件操作
        file_operations_demo(test_dir);

        // 6. 错误处理
        error_handling_demo(test_dir);

        // 7. 其他功能
        misc_demo(test_dir);

        // 8. 对比总结
        comparison_summary();

    } catch (const std::exception& e) {
        std::cerr << "发生未预期的错误: " << e.what() << std::endl;
        return 1;
    }

    // 清理测试目录
    std::cout << "\n--- 清理测试数据 ---" << std::endl;
    fs::remove_all(test_dir);
    std::cout << "测试目录已清理" << std::endl;

    std::cout << "\n========== Demo 结束 ==========" << std::endl;
    return 0;
}
