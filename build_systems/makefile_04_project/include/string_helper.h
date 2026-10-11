/*
 * 示例4：字符串辅助工具头文件
 */
#ifndef STRING_HELPER_H
#define STRING_HELPER_H

#include <string>
#include <vector>

namespace utils {

// 字符串分割
std::vector<std::string> split(const std::string& str, char delimiter);

// 字符串拼接
std::string join(const std::vector<std::string>& parts, const std::string& separator);

// 去除首尾空白
std::string trim(const std::string& str);

// 转大写
std::string to_upper(const std::string& str);

// 转小写
std::string to_lower(const std::string& str);

} // namespace utils

#endif // STRING_HELPER_H
