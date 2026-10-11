/*
 * 示例4：计算器模块实现
 */
#include "calculator.h"
#include <stdexcept>

namespace math {

Calculator::Calculator() : count_(0) {}

Calculator::~Calculator() {}

double Calculator::add(double a, double b) {
    count_++;
    return a + b;
}

double Calculator::subtract(double a, double b) {
    count_++;
    return a - b;
}

double Calculator::multiply(double a, double b) {
    count_++;
    return a * b;
}

double Calculator::divide(double a, double b) {
    count_++;
    if (b == 0) {
        throw std::runtime_error("除数不能为零");
    }
    return a / b;
}

int Calculator::get_count() const {
    return count_;
}

} // namespace math
