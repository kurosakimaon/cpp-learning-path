// ============================================================
//  math_tools.h - 数学工具库（公共头文件）
// ============================================================
//  这是对外暴露的头文件，用户使用我们的库时包含这个。
//  放在 include/mathlib/ 目录下，用命名空间组织。
// ============================================================

#ifndef MATHLIB_MATH_TOOLS_H
#define MATHLIB_MATH_TOOLS_H

#include <vector>

namespace mathlib {

// 计算一系列数的平均值
double average(const std::vector<double>& numbers);

// 计算一系列数的方差
double variance(const std::vector<double>& numbers);

// 计算一系列数的标准差
double standardDeviation(const std::vector<double>& numbers);

// 找出最大值
double maxValue(const std::vector<double>& numbers);

// 找出最小值
double minValue(const std::vector<double>& numbers);

// 排序（升序）
void sortAscending(std::vector<double>& numbers);

} // namespace mathlib

#endif // MATHLIB_MATH_TOOLS_H
