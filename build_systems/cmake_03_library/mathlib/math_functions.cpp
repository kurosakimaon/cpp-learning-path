// ============================================================
//  math_functions.cpp - 数学库实现
// ============================================================

#include "math_functions.h"

namespace mathlib {

// 平方
double square(double x) {
    return x * x;
}

// 立方
double cube(double x) {
    return x * x * x;
}

// 幂运算
double power(double x, int n) {
    if (n == 0) return 1.0;
    if (n < 0) {
        return 1.0 / power(x, -n);
    }

    double result = 1.0;
    for (int i = 0; i < n; ++i) {
        result *= x;
    }
    return result;
}

// 牛顿迭代法求平方根
double sqrt_newton(double x) {
    if (x < 0) return -1.0;  // 负数没有实数平方根
    if (x == 0) return 0;

    double guess = x / 2.0;          // 初始猜测
    double epsilon = 1e-10;          // 精度
    int maxIterations = 1000;        // 最大迭代次数

    for (int i = 0; i < maxIterations; ++i) {
        double next = (guess + x / guess) / 2.0;
        if ((next - guess) < epsilon && (guess - next) < epsilon) {
            return next;
        }
        guess = next;
    }
    return guess;
}

// 圆的面积
const double PI = 3.14159265358979323846;

double circleArea(double radius) {
    return PI * radius * radius;
}

} // namespace mathlib
