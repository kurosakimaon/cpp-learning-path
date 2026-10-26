// ============================================================
//  internal.h - 库内部使用的工具函数
// ============================================================
//  这个头文件放在 src/ 下，通过 target_include_directories 的
//  PRIVATE 配置引入，外部目标访问不到。
// ============================================================

#ifndef MATHUTILS_INTERNAL_H
#define MATHUTILS_INTERNAL_H

namespace mathutils {
namespace internal {

// 阶乘的递归实现（仅供库内部使用）
inline long long factorial_impl(int n) {
    return n <= 1 ? 1LL : static_cast<long long>(n) * factorial_impl(n - 1);
}

} // namespace internal
} // namespace mathutils

#endif // MATHUTILS_INTERNAL_H
