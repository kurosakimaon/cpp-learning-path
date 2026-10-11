// ============================================================
//  math.h - 数学计算函数头文件
// ============================================================

#ifndef MATH_H
#define MATH_H

// 计算阶乘 n!
// 比如 factorial(5) = 1*2*3*4*5 = 120
int factorial(int n);

// 计算斐波那契数列第 n 项
// 0, 1, 1, 2, 3, 5, 8, 13, 21, ...
int fibonacci(int n);

// 判断一个数是不是质数
bool isPrime(int n);

// 求两个数的最大公约数
int gcd(int a, int b);

#endif // MATH_H
