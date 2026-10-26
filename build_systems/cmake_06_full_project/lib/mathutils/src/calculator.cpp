// ============================================================
//  calculator.cpp - 计算器库实现
// ============================================================

#include "mathutils/calculator.h"
#include "internal.h"

#include <stdexcept>

namespace mathutils {

double Calculator::add(double a, double b) const {
    return a + b;
}

double Calculator::subtract(double a, double b) const {
    return a - b;
}

double Calculator::multiply(double a, double b) const {
    return a * b;
}

double Calculator::divide(double a, double b) const {
    if (b == 0.0) {
        throw std::runtime_error("除数不能为 0");
    }
    return a / b;
}

long long Calculator::factorial(int n) const {
    if (n < 0) {
        throw std::invalid_argument("阶乘的参数不能为负数");
    }
    return internal::factorial_impl(n);
}

} // namespace mathutils
