/*
 * 字符串处理库 —— 再做一个库，演示链接多个库
 */

#include "string_utils.h"
#include <cstring>
#include <cctype>

namespace strutils {

// 字符串长度
size_t length(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

// 字符串转大写
void toUpper(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}

// 字符串转小写
void toLower(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}

// 字符串反转
void reverse(char* str) {
    size_t len = length(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

// 判断回文字符串
bool isPalindrome(const char* str) {
    size_t len = length(str);
    for (size_t i = 0; i < len / 2; i++) {
        if (str[i] != str[len - 1 - i]) {
            return false;
        }
    }
    return true;
}

} // namespace strutils
