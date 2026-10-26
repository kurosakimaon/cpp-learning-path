/**
 * @file string_view_filesystem.cpp
 * @brief C++17 std::string_view 与 std::filesystem 高级用法
 *
 * 知识点覆盖：
 * 1. std::string_view 零拷贝字符串处理：原理、子串、查找、比较
 *    - 生命周期陷阱、与 string 的互转、作为函数参数的最佳实践
 * 2. std::filesystem 路径操作：path 类、路径拼接、分解、规范化
 * 3. 目录遍历：directory_iterator / recursive_directory_iterator
 * 4. 文件状态查询：file_size、last_write_time、权限、类型判断
 * 5. 文件操作：创建/删除/复制/移动/重命名
 * 6. 错误处理：error_code vs 异常、路径的跨平台注意事项
 *
 * 编译命令: g++ -std=c++17 -Wall -Wextra -pedantic string_view_filesystem.cpp -o string_view_filesystem
 *          (部分系统需要链接 -lstdc++fs 或 -lc++fs)
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <fstream>
#include <sstream>
#include <optional>

#include <filesystem>

namespace fs = std::filesystem;

// ============================================================
// 第一部分：std::string_view 深度解析
// ============================================================

namespace string_view_demo {

// 1.1 string_view 是什么？
// string_view 是一个非拥有的字符串视图，本质上是 {指针, 长度} 对
// 它指向别人的内存，不负责分配和释放
// 优势：零拷贝、O(1) 子串、无内存分配开销

void demo_basics() {
    std::cout << "===== 1.1 string_view 基础 =====\n";

    // 从 C 字符串构造
    std::string_view sv1 = "Hello, World!";
    std::cout << "sv1 = " << sv1 << "\n";
    std::cout << "sv1.size() = " << sv1.size() << "\n";

    // 从 std::string 构造
    std::string str = "Hello, C++17!";
    std::string_view sv2 = str;  // 隐式转换
    std::cout << "sv2 = " << sv2 << "\n";

    // 从指针 + 长度构造
    const char* cstr = "abcdefgh";
    std::string_view sv3(cstr, 3);  // 只看前3个字符
    std::cout << "sv3 (前3个字符) = " << sv3 << "\n";

    // data() 和 size()
    std::cout << "sv1.data() = " << static_cast<const void*>(sv1.data())
              << ", sv1.size() = " << sv1.size() << "\n";
}

// 1.2 零拷贝子串（核心优势）
void demo_substring() {
    std::cout << "\n===== 1.2 零拷贝子串 =====\n";

    std::string original = "The quick brown fox jumps over the lazy dog";
    std::string_view sv = original;

    // substr: O(1) 时间，无内存分配
    auto first_word = sv.substr(0, 3);   // "The"
    auto second_word = sv.substr(4, 5);  // "quick"
    auto last_part = sv.substr(16);      // 从第16位到结尾

    std::cout << "原字符串: " << original << "\n";
    std::cout << "第一个词: " << first_word << " (零拷贝)\n";
    std::cout << "第二个词: " << second_word << " (零拷贝)\n";
    std::cout << "后半部分: " << last_part << " (零拷贝)\n";

    // 对比 std::string 的 substr（会分配内存、拷贝数据）
    auto str_sub = original.substr(0, 3);  // 产生新的 string，有拷贝
    std::cout << "\nstring::substr 会产生新字符串（有拷贝开销）\n";
    std::cout << "string_view::substr 只是调整指针和长度（零开销）\n";
}

// 1.3 string_view 作为函数参数的最佳实践
// 传参时用 string_view 代替 const std::string& 或 const char*
// 优点：
//   - 可以接受 std::string、const char*、字符串字面量
//   - 不需要隐式构造 string（避免内存分配）
//   - 统一接口

// 传统方式1：const char*（不支持 string 直接传，需要 .c_str()）
// int count_vowels(const char* s);

// 传统方式2：const string&（传字面量时会构造临时 string）
// int count_vowels(const std::string& s);

// C++17 最佳方式：string_view
int count_vowels(std::string_view s) {
    int count = 0;
    for (char c : s) {
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
            ++count;
        }
    }
    return count;
}

void demo_parameter_pattern() {
    std::cout << "\n===== 1.3 作为函数参数的最佳实践 =====\n";

    // 可以接受各种形式
    std::cout << "count_vowels(\"Hello World\") = "
              << count_vowels("Hello World") << "\n";

    std::string str = "Beautiful Day";
    std::cout << "count_vowels(str) = "
              << count_vowels(str) << "\n";

    std::string_view sv = "Programming";
    std::cout << "count_vowels(sv) = "
              << count_vowels(sv) << "\n";

    std::cout << "\n优势：一种参数类型，接受多种字符串表示，零拷贝\n";
}

// 1.4 字符串解析实战：分割路径
// 注意：返回的每个 string_view 都指向 path 内部，调用方必须保证 path 比
// 返回的视图活得更久。下面这种用法会让所有视图悬空：
//   auto parts = split_path(std::string("/a/b"));  // 临时 string 语句结束即销毁
std::vector<std::string_view> split_path(std::string_view path) {
    std::vector<std::string_view> components;
    size_t start = 0;

    while (start < path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string_view::npos) {
            components.push_back(path.substr(start));
            break;
        }
        if (end > start) {  // 跳过空段（如连续的 //）
            components.push_back(path.substr(start, end - start));
        }
        start = end + 1;
    }

    return components;
}

void demo_string_parsing() {
    std::cout << "\n===== 1.4 字符串解析实战 =====\n";

    std::string path = "/usr/local/include/c++/17.0.0";
    auto components = split_path(path);

    std::cout << "路径: " << path << "\n";
    std::cout << "分割结果 (" << components.size() << " 个组件):\n";
    for (size_t i = 0; i < components.size(); ++i) {
        std::cout << "  [" << i << "] " << components[i] << "\n";
    }
    std::cout << "（所有子串都是零拷贝，指向原始 path 的内存）\n";
}

// 1.5 生命周期陷阱（最重要的注意事项）
// string_view 不拥有数据，必须确保指向的内存比它更长寿

std::string_view bad_function() {
    std::string local = "hello";
    return local;  // 严重错误！返回指向局部变量的 string_view
                    // local 在函数返回时被销毁，string_view 变成悬空指针
}

void demo_lifetime_traps() {
    std::cout << "\n===== 1.5 生命周期陷阱 =====\n";

    std::cout << "陷阱1：返回局部 string 的 string_view\n";
    std::cout << "  std::string_view bad() {\n";
    std::cout << "    std::string s = \"hello\";\n";
    std::cout << "    return s;  // 危险！s 会被销毁\n";
    std::cout << "  }\n\n";

    std::cout << "陷阱2：用临时 string 构造 string_view\n";
    std::cout << "  std::string_view sv = std::string(\"temp\");\n";
    std::cout << "  // 临时对象在语句结束后销毁，sv 悬空\n\n";

    std::cout << "陷阱3：string 被修改后，string_view 可能失效\n";
    std::string s = "hello";
    std::string_view sv = s;
    std::cout << "  初始: sv = " << sv << "\n";
    s += " world";  // 追加可能重新分配，使 sv 指向已释放的旧缓冲区
    std::cout << "  s += \"world\" 后，sv 可能已失效（标准不保证仍然有效）\n";
    std::cout << "  （本机即使看起来能读，也不要依赖修改后的 string_view）\n\n";

    std::cout << "安全的使用场景：\n";
    std::cout << "  1. 作为函数参数（只在函数内部使用）\n";
    std::cout << "  2. 作为局部变量（指向更长寿的对象）\n";
    std::cout << "  3. 指向字符串字面量（永远有效）\n";
    std::cout << "  4. 类成员时要特别小心（考虑所有权）\n";
}

// 1.6 string_view 与 string 的互转
void demo_conversions() {
    std::cout << "\n===== 1.6 string_view 与 string 互转 =====\n";

    // string -> string_view：隐式转换（安全）
    std::string s = "hello";
    std::string_view sv = s;  // OK，但仅限 s 存活且不被修改期间
    std::cout << "string -> string_view: 隐式转换（注意 s 的生命周期）\n";

    // string_view -> string：显式构造（有拷贝）
    std::string s2(sv);  // OK，显式构造，会拷贝数据
    std::cout << "string_view -> string: 显式构造，有拷贝\n";

    // 不能直接拼接 string_view 和 string
    // std::string result = sv + s;  // 错误！没有 operator+
    std::string result = std::string(sv) + s;  // OK，但有额外拷贝
    std::cout << "拼接需要显式转换: string(sv) + s\n";
}

// 1.7 性能对比
void demo_performance() {
    std::cout << "\n===== 1.7 性能对比 =====\n";

    const int iterations = 1'000'000;
    std::string long_str(1000, 'x');

    // string 参数（可能产生临时对象）
    auto start1 = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        // 模拟一个接受 string 的函数
        auto func = [](const std::string& s) { return s.size(); };
        volatile size_t s = func(long_str.substr(0, 100));
        (void)s;
    }
    auto end1 = std::chrono::steady_clock::now();
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    // string_view 参数（零拷贝）
    auto start2 = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto func = [](std::string_view s) { return s.size(); };
        std::string_view sv = long_str;
        volatile size_t s = func(sv.substr(0, 100));
        (void)s;
    }
    auto end2 = std::chrono::steady_clock::now();
    auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "string + substr: " << t1.count() << " ms\n";
    std::cout << "string_view + substr: " << t2.count() << " ms\n";
    if (t1.count() > 0 && t2.count() > 0) {
        std::cout << "性能差异约 " << (t1.count() * 1.0 / t2.count()) << "x\n";
    } else {
        std::cout << "（耗时太短，无法计算稳定的倍率）\n";
    }
}

} // namespace string_view_demo


// ============================================================
// 第二部分：std::filesystem 路径操作
// ============================================================

namespace fs_path {

// 2.1 path 类的基本操作
void demo_path_basics() {
    std::cout << "\n===== 2.1 path 类基础 =====\n";

    // 构造路径
    fs::path p1 = "/usr/local/bin";
    fs::path p2 = "C:\\Program Files\\App";  // Windows 风格
    fs::path p3 = "folder/file.txt";         // 相对路径

    std::cout << "p1 = " << p1 << "\n";
    std::cout << "p2 = " << p2 << "\n";
    std::cout << "p3 = " << p3 << "\n";

    // 路径拼接（使用 /= 或 / 运算符）
    fs::path base = "/home/user";
    fs::path full = base / "documents" / "report.pdf";
    std::cout << "base / documents / report.pdf = " << full << "\n";

    // 追加（使用 +=）
    fs::path p = "file";
    p += ".txt";
    std::cout << "file + .txt = " << p << "\n";
}

// 2.2 路径分解
void demo_path_decomposition() {
    std::cout << "\n===== 2.2 路径分解 =====\n";

    fs::path p = "/home/user/docs/report.pdf";

    std::cout << "路径: " << p << "\n";
    std::cout << "  root_name(): " << p.root_name() << "\n";
    std::cout << "  root_directory(): " << p.root_directory() << "\n";
    std::cout << "  root_path(): " << p.root_path() << "\n";
    std::cout << "  relative_path(): " << p.relative_path() << "\n";
    std::cout << "  parent_path(): " << p.parent_path() << "\n";
    std::cout << "  filename(): " << p.filename() << "\n";
    std::cout << "  stem(): " << p.stem() << "\n";
    std::cout << "  extension(): " << p.extension() << "\n";

    // 多组件路径
    std::cout << "\n路径组件遍历:\n";
    int i = 0;
    for (const auto& component : p) {
        std::cout << "  [" << i++ << "] " << component << "\n";
    }
}

// 2.3 路径规范化与查询
void demo_path_normalization() {
    std::cout << "\n===== 2.3 路径规范化 =====\n";

    fs::path messy = "/home/./user/../user/docs/./report.pdf";

    std::cout << "原始路径: " << messy << "\n";
    std::cout << "lexically_normal: " << messy.lexically_normal() << "\n";
    std::cout << "lexically_relative(to /home/user): "
              << messy.lexically_relative("/home/user") << "\n";

    // 路径比较
    fs::path p1 = "/usr/local/bin";
    fs::path p2 = "/usr/local/bin/../bin";
    std::cout << "\np1 = " << p1 << "\n";
    std::cout << "p2 = " << p2 << "\n";
    std::cout << "p1 == p2 ? " << std::boolalpha << (p1 == p2) << "\n";
    std::cout << "p1.lexically_normal() == p2.lexically_normal() ? "
              << (p1.lexically_normal() == p2.lexically_normal()) << "\n";

    // 路径前缀判断
    fs::path base = "/usr/local";
    fs::path sub = "/usr/local/bin/gcc";
    std::cout << "\nbase = " << base << "\n";
    std::cout << "sub = " << sub << "\n";
    // 按路径组件比较，而不是按字符串前缀比较。
    // 字符串前缀会把 "/usr/locally" 误判为以 "/usr/local" 开头。
    auto mismatch = std::mismatch(base.begin(), base.end(),
                                  sub.begin(), sub.end());
    bool startsWith = (mismatch.first == base.end());
    std::cout << "sub 以 base 开头? " << std::boolalpha << startsWith << "\n";
}

// 2.4 跨平台注意事项
void demo_cross_platform() {
    std::cout << "\n===== 2.4 跨平台注意事项 =====\n";

    std::cout << "path 存储的是原生格式，不会自动替换已有分隔符:\n";
    std::cout << "  - Windows 的 API 同时接受 / 和 \\ 作为分隔符；\n";
    std::cout << "    path 构造/string() 不会把所有 / 自动改写成 \\\n";
    std::cout << "  - POSIX 上反斜杠 \\ 可以是普通文件名字符，不是分隔符\n";
    std::cout << "  - 要统一成当前平台的原生分隔符，可调用 make_preferred()\n";
    std::cout << "  - 推荐在源码中使用 / 作为通用分隔符（可移植）\n\n";

    std::cout << "常用路径:\n";
    std::cout << "  current_path(): " << fs::current_path() << "\n";
    std::cout << "  temp_directory_path(): " << fs::temp_directory_path() << "\n";

    // 注意：以下路径可能不存在，仅作演示
    // std::cout << "  home 路径需自己实现 (getenv(\"HOME\") 或 SHGetKnownFolderPath)\n";
}

} // namespace fs_path


// ============================================================
// 第三部分：目录遍历
// ============================================================

namespace fs_directory {

// 3.1 单层目录遍历
void demo_directory_iteration() {
    std::cout << "\n===== 3.1 目录遍历 =====\n";

    fs::path dir = fs::current_path();

    std::cout << "遍历目录: " << dir << "\n";
    std::cout << "------------------------\n";

    int file_count = 0;
    int dir_count = 0;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_directory()) {
            std::cout << "  [DIR]  " << entry.path().filename() << "\n";
            ++dir_count;
        } else if (entry.is_regular_file()) {
            std::cout << "  [FILE] " << entry.path().filename()
                      << " (" << entry.file_size() << " bytes)\n";
            ++file_count;
        } else {
            std::cout << "  [OTHER] " << entry.path().filename() << "\n";
        }
    }

    std::cout << "------------------------\n";
    std::cout << "共 " << dir_count << " 个目录, " << file_count << " 个文件\n";
}

// 3.2 递归目录遍历
void demo_recursive_iteration() {
    std::cout << "\n===== 3.2 递归目录遍历 =====\n";

    fs::path dir = fs::current_path();
    std::cout << "递归遍历: " << dir << "\n";

    int total_files = 0;
    int total_dirs = 0;

    // 注意：递归遍历可能很深，这里用迭代器的 depth() 做限制
    for (auto it = fs::recursive_directory_iterator(
             dir,
             fs::directory_options::skip_permission_denied
         );
         it != fs::recursive_directory_iterator(); ++it) {
        const auto& entry = *it;
        if (entry.is_directory()) {
            ++total_dirs;
            // 打印缩进表示深度
            std::string indent(it.depth() * 2, ' ');
            std::cout << indent << "[DIR] " << entry.path().filename() << "\n";
        } else {
            ++total_files;
        }

        // 限制深度：到达第 2 层后，不再递归进入更深的子目录
        if (it.depth() >= 2) {
            it.disable_recursion_pending();
        }
    }

    std::cout << "总计: " << total_dirs << " 个目录, "
              << total_files << " 个文件\n";
}

// 3.3 查找特定类型文件
std::vector<fs::path> find_files_by_extension(
    const fs::path& root, std::string_view ext)
{
    std::vector<fs::path> results;

    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(
             root,
             fs::directory_options::skip_permission_denied,
             ec
         )) {
        if (ec) {
            std::cerr << "遍历错误: " << ec.message() << "\n";
            break;
        }
        if (entry.is_regular_file() && entry.path().extension() == ext) {
            results.push_back(entry.path());
        }
    }

    return results;
}

void demo_find_files() {
    std::cout << "\n===== 3.3 按扩展名查找文件 =====\n";

    fs::path dir = fs::current_path();
    auto cpp_files = find_files_by_extension(dir, ".cpp");

    std::cout << "在 " << dir << " 下找到 " << cpp_files.size() << " 个 .cpp 文件:\n";
    for (const auto& f : cpp_files) {
        std::cout << "  " << f.filename() << "\n";
    }
}

} // namespace fs_directory


// ============================================================
// 第四部分：文件状态查询与操作
// ============================================================

namespace fs_file_ops {

// 4.1 文件状态查询
void demo_file_status() {
    std::cout << "\n===== 4.1 文件状态查询 =====\n";

    // 以当前可执行文件或某个已知文件为例
    fs::path p = fs::current_path() / "dummy.txt";

    // 创建一个临时文件用于测试
    {
        std::ofstream ofs(p);
        ofs << "Hello, filesystem!" << std::endl;
    }

    std::cout << "文件: " << p << "\n";
    std::cout << "  exists(): " << std::boolalpha << fs::exists(p) << "\n";
    std::cout << "  is_regular_file(): " << fs::is_regular_file(p) << "\n";
    std::cout << "  is_directory(): " << fs::is_directory(p) << "\n";
    std::cout << "  file_size(): " << fs::file_size(p) << " bytes\n";

    // 最后修改时间
    auto ftime = fs::last_write_time(p);
    // C++20 之前 last_write_time 的时间点类型不易转换为人类可读格式
    // 这里打印内部时钟计数，只演示 API 和类型
    std::cout << "  last_write_time(): " << ftime.time_since_epoch().count()
              << " (文件时钟计数)\n";

    // 文件权限
    auto perms = fs::status(p).permissions();
    std::cout << "  permissions: "
              << ((perms & fs::perms::owner_read) != fs::perms::none ? "r" : "-")
              << ((perms & fs::perms::owner_write) != fs::perms::none ? "w" : "-")
              << ((perms & fs::perms::owner_exec) != fs::perms::none ? "x" : "-")
              << "\n";

    // 清理
    fs::remove(p);
}

// 4.2 文件操作：创建、复制、删除、重命名
void demo_file_operations() {
    std::cout << "\n===== 4.2 文件操作 =====\n";

    fs::path test_dir = fs::temp_directory_path() / "cpp17_fs_test";

    std::cout << "测试目录: " << test_dir << "\n";

    // 创建目录
    std::error_code ec;
    fs::create_directory(test_dir, ec);
    if (ec) {
        std::cerr << "创建目录失败: " << ec.message() << "\n";
        return;
    }
    std::cout << "  create_directory: OK\n";

    // 创建多级目录
    fs::path subdir = test_dir / "a" / "b" / "c";
    fs::create_directories(subdir, ec);
    std::cout << "  create_directories: "
              << (ec ? "失败" : "OK") << "\n";

    // 创建文件
    fs::path file1 = test_dir / "file1.txt";
    {
        std::ofstream ofs(file1);
        ofs << "test content";
    }
    std::cout << "  创建文件: OK\n";

    // 复制文件
    fs::path file2 = test_dir / "file2.txt";
    fs::copy_file(file1, file2, ec);
    std::cout << "  copy_file: "
              << (ec ? "失败" : "OK") << "\n";

    // 重命名
    fs::path file3 = test_dir / "file3_renamed.txt";
    fs::rename(file2, file3, ec);
    std::cout << "  rename: "
              << (ec ? "失败" : "OK") << "\n";

    // 删除文件
    fs::remove(file3, ec);
    std::cout << "  remove: "
              << (ec ? "失败" : "OK") << "\n";

    // 删除整个目录（递归）
    fs::remove_all(test_dir, ec);
    std::cout << "  remove_all (清理测试目录): "
              << (ec ? "失败" : "OK") << "\n";
}

// 4.3 错误处理：异常 vs error_code
void demo_error_handling() {
    std::cout << "\n===== 4.3 错误处理 =====\n";

    // 方式1：异常方式（默认）
    std::cout << "方式1：异常处理\n";
    try {
        (void)fs::file_size("/nonexistent/path/file.txt");
    } catch (const fs::filesystem_error& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
        std::cout << "  错误码: " << e.code() << "\n";
    }

    // 方式2：error_code 方式（不抛异常）
    std::cout << "方式2：error_code\n";
    std::error_code ec;
    auto size = fs::file_size("/nonexistent/path/file.txt", ec);
    if (ec) {
        std::cout << "  错误: " << ec.message() << "\n";
        std::cout << "  返回值: " << size << " (无意义)\n";
    }

    std::cout << "\n选择建议:\n";
    std::cout << "  - 预期会发生的错误（如文件不存在）: 用 error_code\n";
    std::cout << "  - 不应该发生的错误（如权限不足）: 用异常\n";
}

} // namespace fs_file_ops


// ============================================================
// 第五部分：综合示例 - 目录大小统计工具
// ============================================================

namespace dir_size_tool {

struct DirectoryStats {
    uintmax_t total_size = 0;
    size_t file_count = 0;
    size_t dir_count = 0;
    std::map<std::string, uintmax_t> by_extension;  // 按扩展名统计
};

DirectoryStats analyze_directory(const fs::path& root) {
    DirectoryStats stats;

    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(
             root,
             fs::directory_options::skip_permission_denied,
             ec
         )) {
        if (ec) {
            std::cerr << "警告: 无法访问 " << entry.path() << ": "
                      << ec.message() << "\n";
            ec.clear();
            continue;
        }

        if (entry.is_directory()) {
            ++stats.dir_count;
        } else if (entry.is_regular_file()) {
            ++stats.file_count;

            std::error_code size_ec;
            uintmax_t size = entry.file_size(size_ec);
            if (!size_ec) {
                stats.total_size += size;

                std::string ext = entry.path().extension().string();
                stats.by_extension[ext] += size;
            }
        }
    }

    return stats;
}

std::string format_size(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        ++unit;
    }

    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << size << " " << units[unit];
    return oss.str();
}

void demo_dir_size_tool() {
    std::cout << "\n===== 5. 综合示例：目录大小统计 =====\n";

    fs::path dir = fs::current_path();
    std::cout << "分析目录: " << dir << "\n\n";

    auto stats = analyze_directory(dir);

    std::cout << "统计结果:\n";
    std::cout << "  总大小: " << format_size(stats.total_size) << "\n";
    std::cout << "  文件数: " << stats.file_count << "\n";
    std::cout << "  目录数: " << stats.dir_count << "\n";

    if (!stats.by_extension.empty()) {
        std::cout << "\n按扩展名统计 (Top 5):\n";
        std::vector<std::pair<std::string, uintmax_t>> ext_list(
            stats.by_extension.begin(), stats.by_extension.end());
        std::sort(ext_list.begin(), ext_list.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        int count = 0;
        for (const auto& [ext, size] : ext_list) {
            if (count++ >= 5) break;
            std::cout << "  " << (ext.empty() ? "(无扩展名)" : ext)
                      << ": " << format_size(size) << "\n";
        }
    }
}

} // namespace dir_size_tool


// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "C++17 string_view 与 filesystem 高级用法\n";
    std::cout << "============================================\n";

    // string_view 部分
    string_view_demo::demo_basics();
    string_view_demo::demo_substring();
    string_view_demo::demo_parameter_pattern();
    string_view_demo::demo_string_parsing();
    string_view_demo::demo_lifetime_traps();
    string_view_demo::demo_conversions();
    string_view_demo::demo_performance();

    // filesystem 部分
    fs_path::demo_path_basics();
    fs_path::demo_path_decomposition();
    fs_path::demo_path_normalization();
    fs_path::demo_cross_platform();

    fs_directory::demo_directory_iteration();
    fs_directory::demo_recursive_iteration();
    fs_directory::demo_find_files();

    fs_file_ops::demo_file_status();
    fs_file_ops::demo_file_operations();
    fs_file_ops::demo_error_handling();

    dir_size_tool::demo_dir_size_tool();

    std::cout << "\n============================================\n";
    std::cout << "所有演示完成！\n";
    std::cout << "============================================\n";

    return 0;
}
