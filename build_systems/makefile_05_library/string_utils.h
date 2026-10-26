/*
 * 字符串处理库 —— 对外接口头文件
 * 只放声明，实现在 string_utils.cpp 里
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <cstddef>

namespace strutils {

size_t length(const char* str);

void toUpper(char* str);
void toLower(char* str);
void reverse(char* str);
bool isPalindrome(const char* str);

} // namespace strutils

#endif // STRING_UTILS_H
