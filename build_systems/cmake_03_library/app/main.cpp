// ============================================================
//  main.cpp - 使用数学库的应用程序
// ============================================================

#include <iostream>
#include <iomanip>
#include "math_functions.h"   // 引入数学库的头文件

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CMake 库链接示例 - 数学库演示" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // 测试平方和立方
    std::cout << "【平方与立方】" << std::endl;
    double x = 5.0;
    std::cout << "  " << x << " 的平方 = " << mathlib::square(x) << std::endl;
    std::cout << "  " << x << " 的立方 = " << mathlib::cube(x) << std::endl;
    std::cout << std::endl;

    // 测试幂运算
    std::cout << "【幂运算】" << std::endl;
    std::cout << "  2^10 = " << mathlib::power(2, 10) << std::endl;
    std::cout << "  3^4 = " << mathlib::power(3, 4) << std::endl;
    std::cout << "  2^(-3) = " << std::fixed << std::setprecision(6)
              << mathlib::power(2, -3) << std::endl;
    std::cout << std::endl;

    // 测试平方根
    std::cout << "【平方根（牛顿迭代法）】" << std::endl;
    std::cout << "  sqrt(2) = " << std::fixed << std::setprecision(10)
              << mathlib::sqrt_newton(2) << std::endl;
    std::cout << "  sqrt(144) = " << mathlib::sqrt_newton(144) << std::endl;
    std::cout << "  sqrt(3) = " << std::fixed << std::setprecision(10)
              << mathlib::sqrt_newton(3) << std::endl;
    std::cout << std::endl;

    // 测试圆面积
    std::cout << "【圆面积计算】" << std::endl;
    double r = 1.0;
    std::cout << "  半径 = " << r << "，面积 = "
              << std::fixed << std::setprecision(6)
              << mathlib::circleArea(r) << std::endl;
    r = 5.0;
    std::cout << "  半径 = " << r << "，面积 = "
              << mathlib::circleArea(r) << std::endl;
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "  演示完毕！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
