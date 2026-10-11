/*
 * 字符串处理库实现
 */

#include "string_utils.h"

namespace strutils {

size_t length(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void toUpper(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = str[i] - 'a' + 'A';
    }
}

void toLower(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] - 'A' + 'a';
    }
}

void reverse(char* str) {
    size_t len = length(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

bool isPalindrome(const char* str) {
    size_t len = length(str);
    for (size_t i = 0; i < len / 2; i++) {
        if (str[i] != str[len - 1 - i]) return false;
    }
    return true;
}

} // namespace strutils
