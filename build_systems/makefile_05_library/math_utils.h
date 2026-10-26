/*
 * 数学计算库 —— 对外接口头文件
 * 只放声明，实现在 math_utils.cpp 里
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

namespace mathutils {

int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);

// 整数除法，除数为 0 时返回 0
int divide(int a, int b);

long long factorial(int n);
bool isPrime(int n);

} // namespace mathutils

#endif // MATH_UTILS_H
