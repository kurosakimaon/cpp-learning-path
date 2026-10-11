// ============================================================
//  math_tools.cpp - 数学工具库实现
// ============================================================

#include "mathlib/math_tools.h"
#include "internal_utils.h"   // 内部头文件，只有库内部能用
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace mathlib {

// 计算平均值
double average(const std::vector<double>& numbers) {
    if (numbers.empty()) {
        return 0.0;
    }

    // 用内部的 add 函数来累加，演示 PRIVATE 头文件的用法
    double sum = 0.0;
    for (double v : numbers) {
        sum = internal::add(sum, v);
    }

    return sum / numbers.size();
}

// 计算方差
double variance(const std::vector<double>& numbers) {
    if (numbers.size() < 2) {
        return 0.0;
    }

    double avg = average(numbers);
    double sumSqDiff = 0.0;

    for (double v : numbers) {
        double diff = internal::subtract(v, avg);
        sumSqDiff += diff * diff;
    }

    return sumSqDiff / numbers.size();
}

// 计算标准差
double standardDeviation(const std::vector<double>& numbers) {
    return std::sqrt(variance(numbers));
}

// 最大值
double maxValue(const std::vector<double>& numbers) {
    if (numbers.empty()) {
        throw std::runtime_error("空数组，没有最大值");
    }
    return *std::max_element(numbers.begin(), numbers.end());
}

// 最小值
double minValue(const std::vector<double>& numbers) {
    if (numbers.empty()) {
        throw std::runtime_error("空数组，没有最小值");
    }
    return *std::min_element(numbers.begin(), numbers.end());
}

// 升序排序
void sortAscending(std::vector<double>& numbers) {
    std::sort(numbers.begin(), numbers.end());
}

} // namespace mathlib
