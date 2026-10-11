/*
 * 数学计算库 —— 用来演示静态库和动态库
 * 这是库的实现文件，会被编译成 .a 或 .so
 */

#include "math_utils.h"

namespace mathutils {

// 加法
int add(int a, int b) {
    return a + b;
}

// 减法
int subtract(int a, int b) {
    return a - b;
}

// 乘法
int multiply(int a, int b) {
    return a * b;
}

// 除法（整数除法，除零返回 0）
int divide(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}

// 阶乘
long long factorial(int n) {
    if (n <= 1) return 1;
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// 判断素数
bool isPrime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; (long long)i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

} // namespace mathutils
