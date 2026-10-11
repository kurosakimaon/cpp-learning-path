/**
 * @file algorithm_demo.cpp
 * @brief Boost.Algorithm 字符串算法与通用算法
 *
 * Boost.Algorithm 提供了很多实用的算法补充，
 * 特别是字符串处理方面，大大提高编码效率。
 *
 * C++ 标准库的算法虽然全，但日常编码中还是有些常用操作没有，
 * 比如字符串 trim、split、大小写转换、判断前缀后缀等。
 * Boost.Algorithm 就是补这些空缺的。
 *
 * 主要内容：
 * 1. 字符串修剪（trim）
 * 2. 大小写转换
 * 3. 字符串判断（starts_with/ends_with/contains 等）
 * 4. 字符串查找（find_all 等）
 * 5. 字符串替换（replace_all/erase_all 等）
 * 6. 字符串分割（split）
 * 7. 字符串连接（join）
 * 8. 其他实用算法（clamp、is_sorted、all_of/any_of/none_of等）
 *
 * 老师傅的话：
 *   别小看这些字符串小工具，写业务代码天天要用。
 *   以前我自己写 trim、split 函数，每个项目写一遍，
 *   还经常写出 bug（空字符串、边界条件……）。
 *   用 Boost.Algorithm，省心又靠谱。
 *   而且 C++20/23 也在逐步把这些东西加进标准库，
 *   说明这些功能确实是刚需。
 *
 * 编译命令: g++ -std=c++17 algorithm_demo.cpp -o algorithm_demo
 * （大部分是纯头文件，无需链接）
 *
 * 头文件：<boost/algorithm/string.hpp>  — 字符串算法大全
 *        <boost/algorithm/clamp.hpp>    — 范围夹紧
 *        <boost/algorithm/cxx11/all_of.hpp> 等 — C++11算法的早期实现
 */

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <numeric>
#include <cctype>

// Boost.Algorithm 字符串算法
#include <boost/algorithm/string.hpp>
// 其他算法
#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/algorithm/hex.hpp>

namespace ba = boost::algorithm;

// ============================================================================
// 1. 字符串修剪（trim）
// ============================================================================
void demo_trim() {
    std::cout << "\n===== 1. 字符串修剪（trim）=====\n";

    std::string s1 = "   Hello, World!   ";
    std::string s2 = "\t\t  前后有空白  \n";
    std::string s3 = "=== 被等号包围 ===";
    std::string s4 = "没有空格的字符串";

    std::cout << "  原始字符串: \"" << s1 << "\"\n\n";

    // trim_left：去掉左边空白
    std::string left_trimmed = s1;
    ba::trim_left(left_trimmed);
    std::cout << "  trim_left:  \"" << left_trimmed << "\"\n";

    // trim_right：去掉右边空白
    std::string right_trimmed = s1;
    ba::trim_right(right_trimmed);
    std::cout << "  trim_right: \"" << right_trimmed << "\"\n";

    // trim：去掉两边空白
    std::string trimmed = s1;
    ba::trim(trimmed);
    std::cout << "  trim:       \"" << trimmed << "\"\n";

    // trim_copy：不修改原字符串，返回修剪后的副本
    // （上面的 trim_left/trim_right/trim 都是 in-place 修改）
    std::string result = ba::trim_copy(s2);
    std::cout << "\n  trim_copy（不修改原串）:\n";
    std::cout << "    原串: \"" << s2 << "\"\n";
    std::cout << "    结果: \"" << result << "\"\n";

    // 自定义修剪的字符（trim_if）
    // 比如去掉两边的等号
    auto is_eq = [](char c) { return c == '='; };
    std::string s3_copy = s3;
    ba::trim_if(s3_copy, is_eq);
    std::cout << "\n  自定义修剪（去掉等号）:\n";
    std::cout << "    原串: \"" << s3 << "\"\n";
    std::cout << "    结果: \"" << s3_copy << "\"\n";

    // 修剪所有空格字符（包括制表符、换行等）
    std::cout << "\n  注意：默认 trim 会去掉所有空白字符：\n";
    std::cout << "    空格 ' '、制表符 '\\t'、换行 '\\n'、回车 '\\r' 等\n";
    std::cout << "    比自己写 trim 函数考虑周全\n";

    std::cout << "\n  实际应用：\n";
    std::cout << "    - 读取用户输入时去掉多余空格\n";
    std::cout << "    - 解析配置文件时修剪每一行\n";
    std::cout << "    - 处理 CSV 数据时清理字段\n";
}

// ============================================================================
// 2. 大小写转换
// ============================================================================
void demo_case_conversion() {
    std::cout << "\n===== 2. 大小写转换 =====\n";

    std::string s = "Hello, World!";
    std::cout << "  原始字符串: " << s << "\n\n";

    // to_upper：转大写
    std::string upper = s;
    ba::to_upper(upper);
    std::cout << "  to_upper:  " << upper << "\n";

    // to_lower：转小写
    std::string lower = s;
    ba::to_lower(lower);
    std::cout << "  to_lower:  " << lower << "\n";

    // _copy 版本（不修改原串）
    std::cout << "\n  to_upper_copy: " << ba::to_upper_copy(s) << "\n";
    std::cout << "  to_lower_copy: " << ba::to_lower_copy(s) << "\n";

    // 不区分大小写的比较
    std::string a = "Hello";
    std::string b = "hello";

    std::cout << "\n  不区分大小写比较:\n";
    std::cout << "    \"" << a << "\" 和 \"" << b << "\"\n";
    std::cout << "    普通比较 == : " << std::boolalpha << (a == b) << "\n";
    std::cout << "    iequals       : " << ba::iequals(a, b) << "\n";
    std::cout << "    iless         : " << ba::iless(a, b) << "\n";

    // 字符串比较的 i 前缀版本（i = case-insensitive）
    std::cout << "\n  不区分大小写的字典序:\n";
    std::cout << "    \"Apple\" < \"banana\" ? "
              << ba::ilexicographical_compare(std::string("Apple"),
                                               std::string("banana"))
              << "\n";

    std::cout << "\n  应用场景：\n";
    std::cout << "    - 用户名/邮箱不区分大小写\n";
    std::cout << "    - 配置项大小写不敏感\n";
    std::cout << "    - 搜索功能忽略大小写\n";
}

// ============================================================================
// 3. 字符串判断（starts_with / ends_with / contains 等）
// ============================================================================
void demo_string_predicates() {
    std::cout << "\n===== 3. 字符串判断 =====\n";

    std::string s = "Hello, World!";
    std::cout << "  字符串: \"" << s << "\"\n\n";

    // starts_with：以...开头
    std::cout << "  starts_with(\"Hello\") : "
              << std::boolalpha << ba::starts_with(s, "Hello") << "\n";
    std::cout << "  starts_with(\"hello\") : "
              << ba::starts_with(s, "hello") << "（区分大小写）\n";
    std::cout << "  istarts_with(\"hello\"): "
              << ba::istarts_with(s, "hello") << "（不区分大小写）\n";

    // ends_with：以...结尾
    std::cout << "\n  ends_with(\"World!\")  : "
              << ba::ends_with(s, "World!") << "\n";
    std::cout << "  ends_with(\"world!\")  : "
              << ba::ends_with(s, "world!") << "（区分大小写）\n";
    std::cout << "  iends_with(\"world!\") : "
              << ba::iends_with(s, "world!") << "（不区分大小写）\n";

    // contains：包含
    std::cout << "\n  contains(\"lo, Wo\")    : "
              << ba::contains(s, "lo, Wo") << "\n";
    std::cout << "  icontains(\"lo, wo\")   : "
              << ba::icontains(s, "lo, wo") << "\n";

    // equals：完全相等（也有 iequals）
    std::cout << "\n  equals(\"Hello, World!\") : "
              << ba::equals(s, "Hello, World!") << "\n";

    // all：所有字符都满足条件
    std::string digits = "12345";
    std::string mixed = "12a45";
    auto is_digit = [](char c) { return std::isdigit(c) != 0; };
    std::cout << "\n  all（所有字符都满足条件）:\n";
    std::cout << "    \"" << digits << "\" 全是数字? "
              << ba::all(digits, is_digit) << "\n";
    std::cout << "    \"" << mixed << "\" 全是数字? "
              << ba::all(mixed, is_digit) << "\n";

    // any / none
    auto is_upper = [](char c) { return std::isupper(c) != 0; };
    std::cout << "\n  any（任意一个满足）: \"" << s << "\" 有大写字母? "
              << ba::any(s, is_upper) << "\n";

    std::string lower_str = "hello";
    std::cout << "  none（没有一个满足）: \"" << lower_str << "\" 没有大写? "
              << ba::none(lower_str, is_upper) << "\n";

    std::cout << "\n  注：C++11 已经有 std::all_of/any_of/none_of，\n";
    std::cout << "     但 Boost 提供了字符串专用的便捷版本\n";
}

// ============================================================================
// 4. 字符串查找
// ============================================================================
void demo_string_find() {
    std::cout << "\n===== 4. 字符串查找 =====\n";

    std::string s = "banana apple orange apple banana";
    std::cout << "  字符串: \"" << s << "\"\n\n";

    // find_first：找第一次出现的位置（其实 string::find 也能做）
    // 但 Boost 提供了更多变体
    auto pos = ba::find_first(s, "apple");
    if (!pos.empty()) {
        std::cout << "  find_first(\"apple\"): "
                  << "位置 " << (pos.begin() - s.begin())
                  << "，内容 \"" << pos << "\"\n";
    }

    // find_last：找最后一次出现
    auto last_pos = ba::find_last(s, "banana");
    if (!last_pos.empty()) {
        std::cout << "  find_last(\"banana\"): "
                  << "位置 " << (last_pos.begin() - s.begin()) << "\n";
    }

    // find_nth：找第 N 次出现
    auto nth_pos = ba::find_nth(s, "apple", 1);  // 第 2 个 apple（从0开始数）
    if (!nth_pos.empty()) {
        std::cout << "  find_nth(\"apple\", 1): "
                  << "位置 " << (nth_pos.begin() - s.begin()) << "\n";
    }

    // find_head：取前 N 个字符
    auto head = ba::find_head(s, 6);
    std::cout << "  find_head(6): \"" << head << "\"\n";

    // find_tail：取后 N 个字符
    auto tail = ba::find_tail(s, 6);
    std::cout << "  find_tail(6): \"" << tail << "\"\n";

    // find_token：找第一个满足条件的字符
    auto first_upper = ba::find_token(s, [](char c) {
        return std::isupper(c) != 0;
    });
    std::cout << "  find_token（第一个大写字母）: "
              << (first_upper.empty() ? "没有" : "找到了")
              << "（示例字符串全小写，所以没有）\n";

    // 实际应用示例：从URL中提取协议和域名
    std::string url = "https://www.example.com/path/to/page";
    std::cout << "\n  实际应用：URL 解析\n";
    std::cout << "    URL: " << url << "\n";

    auto protocol_end = ba::find_first(url, "://");
    if (!protocol_end.empty()) {
        std::string protocol(url.begin(), protocol_end.begin());
        std::cout << "    协议: " << protocol << "\n";
    }

    std::cout << "  提示：Boost 还有更专业的 URL 解析库，\n";
    std::cout << "       这里只是演示字符串查找的用法\n";
}

// ============================================================================
// 5. 替换与删除
// ============================================================================
void demo_replace_erase() {
    std::cout << "\n===== 5. 替换与删除 =====\n";

    std::string s = "I like apple. I like banana. I like apple.";
    std::cout << "  原始: " << s << "\n\n";

    // replace_first：替换第一个
    std::string s1 = s;
    ba::replace_first(s1, "apple", "orange");
    std::cout << "  replace_first(apple→orange):\n    " << s1 << "\n";

    // replace_last：替换最后一个
    std::string s2 = s;
    ba::replace_last(s2, "apple", "grape");
    std::cout << "\n  replace_last(apple→grape):\n    " << s2 << "\n";

    // replace_all：替换所有
    std::string s3 = s;
    ba::replace_all(s3, "like", "love");
    std::cout << "\n  replace_all(like→love):\n    " << s3 << "\n";

    // erase_all：删除所有
    std::string s4 = s;
    ba::erase_all(s4, "I ");
    std::cout << "\n  erase_all(\"I \"):\n    " << s4 << "\n";

    // _copy 版本
    std::string result = ba::replace_all_copy(s, "apple", "XXX");
    std::cout << "\n  replace_all_copy: " << result << "\n";

    // 不区分大小写的替换
    std::string s5 = "Hello HELLO hello";
    std::string s5_copy = s5;
    ba::ireplace_all(s5_copy, "hello", "hi");
    std::cout << "\n  ireplace_all（不区分大小写）:\n";
    std::cout << "    原串: " << s5 << "\n";
    std::cout << "    结果: " << s5_copy << "\n";

    std::cout << "\n  各种变体：\n";
    std::cout << "    replace_first / replace_last / replace_all / replace_nth\n";
    std::cout << "    erase_first   / erase_last   / erase_all   / erase_nth\n";
    std::cout << "    加 i 前缀就是不区分大小写版本（ireplace_all 等）\n";
    std::cout << "    加 _copy 后缀就是返回副本不修改原串\n";
}

// ============================================================================
// 6. 字符串分割（split）
// ============================================================================
void demo_split() {
    std::cout << "\n===== 6. 字符串分割（split）=====\n";

    // 基本用法：按单个字符分割
    std::string csv = "apple,banana,orange,grape";
    std::cout << "  原始: \"" << csv << "\"\n\n";

    std::vector<std::string> fruits;
    ba::split(fruits, csv, ba::is_any_of(","));

    std::cout << "  按逗号分割:\n";
    for (size_t i = 0; i < fruits.size(); i++) {
        std::cout << "    [" << i << "] \"" << fruits[i] << "\"\n";
    }

    // 按多个分隔符分割
    std::string mixed = "apple,banana;orange grape\tmelon";
    std::vector<std::string> items;
    ba::split(items, mixed, ba::is_any_of(",; \t"));

    std::cout << "\n  按多种分隔符（逗号/分号/空格/制表符）分割:\n";
    for (const auto& item : items) {
        std::cout << "    \"" << item << "\"\n";
    }

    // 压缩空字符串（token_compress_on）
    std::string multi_sep = "a,,b;;;c,d";
    std::vector<std::string> result1, result2;

    ba::split(result1, multi_sep, ba::is_any_of(","),
              ba::token_compress_off);  // 默认：保留空字符串
    ba::split(result2, multi_sep, ba::is_any_of(","),
              ba::token_compress_on);   // 压缩：连续分隔符视为一个

    std::cout << "\n  空字符串处理:\n";
    std::cout << "    原串: \"" << multi_sep << "\"\n";
    std::cout << "    token_compress_off（默认）: " << result1.size() << " 个元素\n";
    std::cout << "    token_compress_on:        " << result2.size() << " 个元素\n";

    // 按条件分割（比如按非数字分割，提取数字）
    std::string text = "order_123_item_456_abc_789";
    std::vector<std::string> parts;
    ba::split(parts, text, !ba::is_digit());
    // 注意：!is_digit() 表示"非数字的字符"作为分隔符

    std::cout << "\n  提取字符串中的数字段:\n";
    std::cout << "    原串: " << text << "\n";
    std::cout << "    数字段: ";
    for (const auto& p : parts) {
        if (!p.empty()) std::cout << p << " ";
    }
    std::cout << "\n";

    // 实用的分隔符谓词
    std::cout << "\n  常用分隔谓词:\n";
    std::cout << "    is_any_of(\"chars\")  — 任意给定字符\n";
    std::cout << "    is_space()            — 空白字符\n";
    std::cout << "    is_digit()            — 数字字符\n";
    std::cout << "    is_alpha()            — 字母字符\n";
    std::cout << "    is_punct()            — 标点符号\n";
    std::cout << "    用 ! 取反，比如 !is_digit() 表示非数字\n";

    std::cout << "\n  小技巧：\n";
    std::cout << "    分割后经常要 trim 每个元素，可以这样写：\n";
    std::cout << "    for (auto& item : items) trim(item);\n";
}

// ============================================================================
// 7. 字符串连接（join）
// ============================================================================
void demo_join() {
    std::cout << "\n===== 7. 字符串连接（join）=====\n";

    std::vector<std::string> words = {"Hello", "World", "Boost", "Algorithm"};
    std::list<int> numbers = {1, 2, 3, 4, 5};

    // 基本用法
    std::string joined = ba::join(words, ", ");
    std::cout << "  用 \", \" 连接: " << joined << "\n";

    // 用不同的分隔符
    std::cout << "  用 \" - \" 连接: " << ba::join(words, " - ") << "\n";
    std::cout << "  用空字符串连接:   " << ba::join(words, "") << "\n";

    // 也可以连接非字符串容器（会转成字符串）
    std::cout << "\n  连接数字列表: " << ba::join(numbers, "+") << "\n";

    // join_if：按条件连接
    // 比如只连接长度大于 5 的单词
    auto is_long = [](const std::string& s) { return s.size() > 5; };
    std::string long_words = ba::join_if(words, " | ", is_long);
    std::cout << "\n  只连接长度>5的单词: " << long_words << "\n";

    std::cout << "\n  split 和 join 是一对好搭档：\n";
    std::cout << "    读取数据 → split 解析 → 处理 → join 拼接 → 输出\n";
    std::cout << "    处理 CSV、配置文件、日志分析时天天用\n";
}

// ============================================================================
// 8. 其他实用算法
// ============================================================================
void demo_other_algorithms() {
    std::cout << "\n===== 8. 其他实用算法 =====\n";

    // clamp：把值限制在范围内（C++17 标准库也有了）
    std::cout << "  clamp（限制范围）:\n";
    std::cout << "    clamp(5, 1, 10)  = " << ba::clamp(5, 1, 10) << "\n";
    std::cout << "    clamp(-3, 1, 10) = " << ba::clamp(-3, 1, 10)
              << "（小于下界，取下界）\n";
    std::cout << "    clamp(15, 1, 10) = " << ba::clamp(15, 1, 10)
              << "（大于上界，取上界）\n";
    std::cout << "    clamp(5.5, 0.0, 1.0) = "
              << ba::clamp(5.5, 0.0, 1.0) << "（也支持浮点数）\n";

    // 应用：限制用户输入、限制百分比等
    int user_input = 150;
    int percent = ba::clamp(user_input, 0, 100);
    std::cout << "\n  应用：百分比限制\n";
    std::cout << "    用户输入 " << user_input << "，限制后 " << percent << "%\n";

    // minmax：同时求最小和最大值
    auto mm = ba::minmax(3, 7);
    std::cout << "\n  minmax(3, 7):\n";
    std::cout << "    min = " << mm.get<0>() << "\n";
    std::cout << "    max = " << mm.get<1>() << "\n";

    // 也可以用于容器
    std::vector<int> v = {5, 2, 8, 1, 9, 3, 7};
    auto range_mm = ba::minmax_element(v.begin(), v.end());
    std::cout << "\n  容器 minmax_element:\n";
    std::cout << "    min = " << *range_mm.first << "\n";
    std::cout << "    max = " << *range_mm.second << "\n";

    // hex 编解码
    std::cout << "\n  hex 编解码:\n";
    std::string data = "Hello";
    std::string hex_str;
    ba::hex(data, std::back_inserter(hex_str));
    std::cout << "    \"Hello\" → hex: " << hex_str << "\n";

    std::string decoded;
    ba::unhex(hex_str, std::back_inserter(decoded));
    std::cout << "    hex → 原文: " << decoded << "\n";

    std::cout << "\n  hex 常用于：二进制数据的文本表示、日志打印等\n";

    // 还有很多算法，这里不一一列举
    std::cout << "\n  Boost.Algorithm 还有很多：\n";
    std::cout << "    - 排序相关：sort_subrange、is_sorted 等\n";
    std::cout << "    - 集合操作：set_union、set_intersection 的增强版\n";
    std::cout << "    - 查找算法：find_backward、find_if_not 等\n";
    std::cout << "    - C++11/14/17 算法的回溯实现（旧编译器也能用）\n";
    std::cout << "  用到的时候查文档就行，不用全背\n";
}

// ============================================================================
// 9. 综合实战：日志行解析
// ============================================================================
void demo_log_parsing() {
    std::cout << "\n===== 9. 综合实战：日志行解析 =====\n";

    // 解析这样的日志行：
    // [2024-06-15 14:30:25] [INFO] [UserModule] 登录成功，用户ID=1001
    std::string log_line = "[2024-06-15 14:30:25] [INFO] [UserModule] 登录成功，用户ID=1001";
    std::cout << "  日志行: " << log_line << "\n\n";

    // 第一步：提取方括号中的字段
    // 先去掉首尾的空格
    std::string line = log_line;
    ba::trim(line);

    // 提取所有方括号内的内容（简化版，用 split 模拟）
    std::vector<std::string> parts;
    ba::split(parts, line, ba::is_any_of("[]"), ba::token_compress_on);

    // 去掉空字符串并 trim
    std::vector<std::string> fields;
    for (auto& p : parts) {
        ba::trim(p);
        if (!p.empty()) {
            fields.push_back(p);
        }
    }

    std::cout << "  解析结果:\n";
    if (fields.size() >= 3) {
        std::cout << "    时间: " << fields[0] << "\n";
        std::cout << "    级别: " << fields[1] << "\n";
        std::cout << "    模块: " << fields[2] << "\n";
    }
    if (fields.size() >= 4) {
        std::cout << "    内容: " << fields[3] << "\n";
    }

    // 提取 key=value 形式的参数（如 用户ID=1001）
    // 找到 = 号，提取前后内容
    auto eq_pos = ba::find_last(line, "=");
    if (!eq_pos.empty()) {
        std::string value(eq_pos.end(), line.end());
        std::cout << "\n  提取到参数:\n";
        std::cout << "    值: " << value << "\n";
    }

    // 判断日志级别
    std::string level;
    if (fields.size() >= 2) {
        level = fields[1];
    }
    std::cout << "\n  日志级别判断:\n";
    std::cout << "    是 INFO? " << std::boolalpha
              << ba::iequals(level, "INFO") << "\n";
    std::cout << "    是 ERROR? "
              << ba::iequals(level, "ERROR") << "\n";

    std::cout << "\n  这是日常开发中非常典型的场景：\n";
    std::cout << "    读一行日志 → 解析字段 → 提取信息 → 做判断\n";
    std::cout << "    Boost.Algorithm 让这些操作变得简单优雅\n";
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
    std::cout << "  字符串处理，说难不难，说简单也不简单。\n";
    std::cout << "  每个程序员都写过 trim、split 函数，\n";
    std::cout << "  但自己写的东西，总是漏考虑一些边界情况。\n";
    std::cout << "  空字符串怎么办？连续分隔符怎么办？中文怎么办？\n";
    std::cout << "\n";
    std::cout << "  Boost.Algorithm 里这些小工具，\n";
    std::cout << "  看着不起眼，用起来真香。\n";
    std::cout << "  trim、split、join、replace_all……\n";
    std::cout << "  写业务代码的时候，随手就用了，效率高还不出错。\n";
    std::cout << "\n";
    std::cout << "  好消息是，C++ 标准库也在不断补这些东西：\n";
    std::cout << "  - C++17: std::clamp\n";
    std::cout << "  - C++20: starts_with, ends_with, contains\n";
    std::cout << "  - C++23: contains for string_view\n";
    std::cout << "  说明这些功能确实是刚需。\n";
    std::cout << "\n";
    std::cout << "  建议：\n";
    std::cout << "  1️⃣ 优先用标准库的（编译器支持的话）\n";
    std::cout << "  2️⃣  标准库没有的，用 Boost.Algorithm\n";
    std::cout << "  3️⃣  不要自己重复造轮子，真的容易出 bug\n";
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
    std::cout << "  1. 写一个函数，判断一个字符串是不是合法的邮箱格式？\n";
    std::cout << "     要求：包含 @，@ 前后都有内容，最后有 .xxx 后缀\n";
    std::cout << "     （提示：用 contains、ends_with、split 等）\n";
    std::cout << "\n";
    std::cout << "  2. 写一个函数，把下划线命名法转为驼峰命名法：\n";
    std::cout << "     输入: user_name_and_password\n";
    std::cout << "     输出: userNameAndPassword\n";
    std::cout << "     （提示：split + to_upper_copy + join）\n";
    std::cout << "\n";
    std::cout << "  3. 写一个函数，统计一段文本中每个单词出现的次数：\n";
    std::cout << "     不区分大小写，忽略标点符号\n";
    std::cout << "     （提示：to_lower + split + map 计数）\n";
    std::cout << "============================================================\n";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  Boost.Algorithm 字符串与通用算法\n";
    std::cout << "  提高编码效率的小工具集 —— 小而美，天天用\n";
    std::cout << "============================================================\n";

    demo_trim();
    demo_case_conversion();
    demo_string_predicates();
    demo_string_find();
    demo_replace_erase();
    demo_split();
    demo_join();
    demo_other_algorithms();
    demo_log_parsing();

    laoshifu_says();
    exercises();

    return 0;
}
