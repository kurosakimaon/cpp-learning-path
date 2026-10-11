# ============================================================
#  mathlib 子目录的 CMakeLists.txt
#  用来构建数学库
# ============================================================

# ---- add_library：创建一个库 ----
#
# 格式：add_library(库名  [STATIC|SHARED|MODULE]  源文件...)
#
# STATIC：静态库，编译时把库的代码全部塞进可执行文件里
#         Linux 下生成 .a 文件，Windows 下生成 .lib 文件
#
# SHARED：动态库（共享库），编译时只记个引用，运行时才加载
#         Linux 下生成 .so 文件，Windows 下生成 .dll 文件
#
# MODULE：模块库，跟 SHARED 类似，但主要用于运行时动态加载
#         （比如插件系统），不参与链接
#
# 这里我们先做个静态库 STATIC，简单直接。
#
add_library(mathlib STATIC
    math_functions.cpp
)

# ============================================================
#  target_include_directories：给目标指定头文件目录
# ============================================================
#
# 徒弟你看好了，这是现代 CMake 的写法，比 include_directories 高级。
#
# 区别在哪？
#   include_directories 是全局的，所有目标都能用上。
#   target_include_directories 是针对某个具体目标的。
#
# 为什么 target_include_directories 更好？
#   因为它更"干净"。这个库需要什么头文件路径，就给它自己加，
#   不会影响别的目标。大项目里这点尤其重要。
#
# PUBLIC/PRIVATE/INTERFACE 是什么？
#   这可是现代 CMake 的精髓，咱先简单说：
#
#   PRIVATE：只有我自己用，别人链接我也不需要知道
#   PUBLIC：我自己用，别人链接我也需要（要传递出去）
#   INTERFACE：我自己不用，但别人链接我需要
#
#   这里用 PUBLIC，因为使用这个库的人（app）也需要包含头文件，
#   所以得把头文件路径"传递"给链接者。
#
#   详细的咱到第 5 讲再细讲，先混个脸熟。
#
target_include_directories(mathlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 解释一下 ${CMAKE_CURRENT_SOURCE_DIR}：
# 就是当前这个 CMakeLists.txt 所在的目录，也就是 mathlib/ 目录。
# 头文件 math_functions.h 就在这个目录里。
