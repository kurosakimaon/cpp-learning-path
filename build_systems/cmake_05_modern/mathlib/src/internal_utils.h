// ============================================================
//  internal_utils.h - 内部工具函数（不对外暴露）
// ============================================================
//  这是库的内部用的头文件，只在库内部使用，
//  不对外暴露给用户。
//  用 PRIVATE 的方式加到 target_include_directories 里。
// ============================================================

#ifndef MATHLIB_INTERNAL_UTILS_H
#define MATHLIB_INTERNAL_UTILS_H

namespace mathlib {
namespace internal {

// 计算两个数的和（内部用的简单函数，演示 PRIVATE 头文件）
double add(double a, double b);

// 计算两个数的差
double subtract(double a, double b);

} // namespace internal
} // namespace mathlib

#endif // MATHLIB_INTERNAL_UTILS_H
