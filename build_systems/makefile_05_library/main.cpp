/*
 * 主程序 —— 链接上面两个库
 * 演示如何使用静态库和动态库
 */

#include <iostream>
#include "math_utils.h"
#include "string_utils.h"

using namespace std;

int main() {
    cout << "========== 库链接演示 ==========" << endl;

    // --- 使用数学库 ---
    cout << "\n【数学库 mathutils】" << endl;
    cout << "  10 + 5 = " << mathutils::add(10, 5) << endl;
    cout << "  10 - 5 = " << mathutils::subtract(10, 5) << endl;
    cout << "  10 * 5 = " << mathutils::multiply(10, 5) << endl;
    cout << "  10 / 3 = " << mathutils::divide(10, 3) << endl;
    cout << "  5! = " << mathutils::factorial(5) << endl;
    cout << "  17 是素数吗？ " << (mathutils::isPrime(17) ? "是" : "否") << endl;

    // --- 使用字符串库 ---
    cout << "\n【字符串库 strutils】" << endl;
    char str[] = "Hello, World!";
    cout << "  原字符串: " << str << endl;
    cout << "  长度: " << strutils::length(str) << endl;

    char buf1[64];
    strcpy(buf1, str);
    strutils::toUpper(buf1);
    cout << "  转大写: " << buf1 << endl;

    char buf2[64];
    strcpy(buf2, str);
    strutils::toLower(buf2);
    cout << "  转小写: " << buf2 << endl;

    char buf3[64];
    strcpy(buf3, str);
    strutils::reverse(buf3);
    cout << "  反转: " << buf3 << endl;

    cout << "  \"level\" 是回文吗？ "
         << (strutils::isPalindrome("level") ? "是" : "否") << endl;

    cout << "\n========== 演示结束 ==========" << endl;
    return 0;
}
