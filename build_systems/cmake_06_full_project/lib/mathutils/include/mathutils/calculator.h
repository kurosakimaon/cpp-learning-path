// ============================================================
//  calculator.h - 计算器库（公共头文件）
// ============================================================
//  这是对外暴露的接口，用户使用本库时包含这个文件。
//  实现细节全部放在 src/ 里，不对外暴露。
// ============================================================

#ifndef MATHUTILS_CALCULATOR_H
#define MATHUTILS_CALCULATOR_H

namespace mathutils {

class Calculator {
public:
    Calculator() = default;

    // 四则运算
    double add(double a, double b) const;
    double subtract(double a, double b) const;
    double multiply(double a, double b) const;

    // 除法：除数为 0 时抛出 std::runtime_error
    double divide(double a, double b) const;

    // 阶乘：n 为负数时抛出 std::invalid_argument
    long long factorial(int n) const;
};

} // namespace mathutils

#endif // MATHUTILS_CALCULATOR_H
