// ============================================================
//  test_calculator.cpp - 计算器单元测试
// ============================================================
//  这里我们不引入第三方测试框架（比如 Google Test），
//  就用最简单的方式写测试，演示 CMake 的测试功能。
//  真实项目里推荐用 Google Test 或者 Catch2。
// ============================================================

#include "mathutils/calculator.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <cmath>

// 简单的测试辅助宏
#define TEST_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "  失败：" #expr \
                      << " (第 " << __LINE__ << " 行)" << std::endl; \
            return 1; \
        } \
    } while(0)

#define TEST_ASSERT_NEAR(a, b, eps) \
    do { \
        if (std::fabs((a) - (b)) > (eps)) { \
            std::cerr << "  失败：" #a " ≈ " #b \
                      << " (实际值：" << (a) << "，期望值：" << (b) << ")" \
                      << " (第 " << __LINE__ << " 行)" << std::endl; \
            return 1; \
        } \
    } while(0)

// 测试加法
int test_add() {
    std::cout << "  测试加法... ";
    mathutils::Calculator calc;
    TEST_ASSERT_NEAR(calc.add(2.0, 3.0), 5.0, 1e-10);
    TEST_ASSERT_NEAR(calc.add(-1.0, 1.0), 0.0, 1e-10);
    TEST_ASSERT_NEAR(calc.add(0.0, 0.0), 0.0, 1e-10);
    std::cout << "通过" << std::endl;
    return 0;
}

// 测试减法
int test_subtract() {
    std::cout << "  测试减法... ";
    mathutils::Calculator calc;
    TEST_ASSERT_NEAR(calc.subtract(5.0, 3.0), 2.0, 1e-10);
    TEST_ASSERT_NEAR(calc.subtract(0.0, 5.0), -5.0, 1e-10);
    std::cout << "通过" << std::endl;
    return 0;
}

// 测试乘法
int test_multiply() {
    std::cout << "  测试乘法... ";
    mathutils::Calculator calc;
    TEST_ASSERT_NEAR(calc.multiply(3.0, 4.0), 12.0, 1e-10);
    TEST_ASSERT_NEAR(calc.multiply(-2.0, 3.0), -6.0, 1e-10);
    TEST_ASSERT_NEAR(calc.multiply(0.0, 100.0), 0.0, 1e-10);
    std::cout << "通过" << std::endl;
    return 0;
}

// 测试除法
int test_divide() {
    std::cout << "  测试除法... ";
    mathutils::Calculator calc;
    TEST_ASSERT_NEAR(calc.divide(10.0, 2.0), 5.0, 1e-10);
    TEST_ASSERT_NEAR(calc.divide(7.0, 2.0), 3.5, 1e-10);

    // 测试除零异常
    bool threw = false;
    try {
        calc.divide(1.0, 0.0);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    TEST_ASSERT(threw);  // 应该抛出异常

    std::cout << "通过" << std::endl;
    return 0;
}

// 测试阶乘
int test_factorial() {
    std::cout << "  测试阶乘... ";
    mathutils::Calculator calc;
    TEST_ASSERT(calc.factorial(0) == 1);
    TEST_ASSERT(calc.factorial(1) == 1);
    TEST_ASSERT(calc.factorial(5) == 120);
    TEST_ASSERT(calc.factorial(10) == 3628800);
    std::cout << "通过" << std::endl;
    return 0;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  单元测试 - Calculator" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    int failed = 0;

    failed += test_add();
    failed += test_subtract();
    failed += test_multiply();
    failed += test_divide();
    failed += test_factorial();

    std::cout << std::endl;
    if (failed == 0) {
        std::cout << "所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << "有 " << failed << " 个测试失败了。" << std::endl;
        return 1;
    }
}
