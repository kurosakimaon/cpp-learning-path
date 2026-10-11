/*
 * 示例4：计算器模块头文件
 * 放在 include 目录下，通过 -I 路径包含
 */
#ifndef CALCULATOR_H
#define CALCULATOR_H

namespace math {

class Calculator {
public:
    Calculator();
    ~Calculator();

    double add(double a, double b);
    double subtract(double a, double b);
    double multiply(double a, double b);
    double divide(double a, double b);

    // 统计计算了多少次
    int get_count() const;

private:
    int count_;  // 计算次数
};

} // namespace math

#endif // CALCULATOR_H
