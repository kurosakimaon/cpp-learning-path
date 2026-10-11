// ================================================================
//  Utils.h —— 工具函数
//  一些通用的辅助函数
// ================================================================
//
//  徒弟啊，每个项目里都会有一些"通用工具函数"，
//  就是那种哪里都可能用到、但又不属于任何一个类的函数。
//
//  这些函数我们统一放到 Utils 里，
//  避免到处重复写相同的代码。
//

#ifndef TODO_CLI_UTILS_H
#define TODO_CLI_UTILS_H

#include <string>
#include <vector>

namespace todo {
namespace utils {

// 字符串转小写
std::string toLower(const std::string& s);

// 字符串转大写
std::string toUpper(const std::string& s);

// 去除字符串首尾空白
std::string trim(const std::string& s);

// 按分隔符分割字符串
std::vector<std::string> split(const std::string& s, char delim);

// 用指定分隔符拼接字符串数组
std::string join(const std::vector<std::string>& parts, const std::string& delim);

// 判断字符串是否以指定前缀开头
bool startsWith(const std::string& s, const std::string& prefix);

// 判断字符串是否以指定后缀结尾
bool endsWith(const std::string& s, const std::string& suffix);

// 获取当前时间的字符串表示
std::string currentTimeString();

} // namespace utils
} // namespace todo

#endif // TODO_CLI_UTILS_H
