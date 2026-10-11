// ============================================================
//  math_functions.h - 数学库头文件
// ============================================================

#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

namespace mathlib {

// 计算平方
double square(double x);

// 计算立方
double cube(double x);

// 计算幂（x 的 n 次方）
double power(double x, int n);

// 计算平方根（用牛顿迭代法实现，不用系统库）
double sqrt_newton(double x);

// 计算圆的面积
double circleArea(double radius);

} // namespace mathlib

#endif // MATH_FUNCTIONS_H
