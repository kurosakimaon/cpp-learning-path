// ============================================================
//  main.cpp - 使用 mathlib 的示例程序
// ============================================================

#include <iostream>
#include <vector>
#include <iomanip>

// 包含 mathlib 的公共头文件
// 因为 mathlib 用 PUBLIC 暴露了 include 目录，
// 所以我们能直接 #include "mathlib/math_tools.h"
#include "mathlib/math_tools.h"

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  现代 CMake 示例 - 统计分析" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // 一组测试数据
    std::vector<double> data = {
        12.5, 18.3, 9.7, 22.1, 15.8,
        30.2, 8.4, 17.6, 25.0, 14.9
    };

    std::cout << "【原始数据】" << std::endl;
    std::cout << "  ";
    for (double v : data) {
        std::cout << v << " ";
    }
    std::cout << std::endl << std::endl;

    // 计算各种统计量
    std::cout << "【统计结果】" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  数据个数：" << data.size() << std::endl;
    std::cout << "  平均值：  " << mathlib::average(data) << std::endl;
    std::cout << "  方差：    " << mathlib::variance(data) << std::endl;
    std::cout << "  标准差：  " << mathlib::standardDeviation(data) << std::endl;
    std::cout << "  最大值：  " << mathlib::maxValue(data) << std::endl;
    std::cout << "  最小值：  " << mathlib::minValue(data) << std::endl;
    std::cout << std::endl;

    // 排序
    std::vector<double> sorted = data;
    mathlib::sortAscending(sorted);
    std::cout << "【升序排列后】" << std::endl;
    std::cout << "  ";
    for (double v : sorted) {
        std::cout << v << " ";
    }
    std::cout << std::endl << std::endl;

    // 试试能不能访问内部头文件？
    // 答案是不能！因为 internal_utils.h 在 src/ 目录，
    // 那个目录是 PRIVATE 的，外部目标访问不到。
    // 你要是不信，把下面这行注释取消试试，编译肯定报错：
    //
    // #include "internal_utils.h"  // 错误！找不到这个头文件
    //

    std::cout << "========================================" << std::endl;
    std::cout << "  演示完毕！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
