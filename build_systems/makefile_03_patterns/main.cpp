/*
 * 示例3：主程序
 * 调用数学工具和字符串工具
 */
#include <iostream>
#include <cstring>
#include "math_utils.h"
#include "string_utils.h"

int main() {
    std::cout << "===== 数学函数演示 =====" << std::endl;
    std::cout << "5的阶乘: " << factorial(5) << std::endl;
    std::cout << "第10个斐波那契数: " << fibonacci(10) << std::endl;
    std::cout << "17是质数吗? " << (is_prime(17) ? "是" : "不是") << std::endl;

    std::cout << std::endl;
    std::cout << "===== 字符串函数演示 =====" << std::endl;
    const char* s = "hello";
    std::cout << "\"" << s << "\" 的长度: " << str_length(s) << std::endl;

    char buf[100];
    strcpy(buf, "level");
    std::cout << "\"level\" 是回文吗? " << (is_palindrome(buf) ? "是" : "不是") << std::endl;

    strcpy(buf, "abcde");
    str_reverse(buf);
    std::cout << "\"abcde\" 反转后: " << buf << std::endl;

    return 0;
}
