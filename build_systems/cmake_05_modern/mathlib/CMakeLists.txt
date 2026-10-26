# ============================================================
#  mathlib 子目录 CMakeLists.txt
#  现代 CMake 写法示范
#  老师傅注释版
# ============================================================

# ---- 创建库目标 ----
add_library(mathlib STATIC
    src/math_tools.cpp
    src/internal_utils.cpp
)

# ============================================================
#  target_include_directories - PUBLIC vs PRIVATE
# ============================================================
#
#  徒弟啊，这里可是现代 CMake 最核心的知识点之一，
#  你可得给我听仔细了！
#
#  头文件目录分两种：
#
#  1. PUBLIC 的头文件目录
#     就是对外暴露的头文件所在的目录。
#     不仅我自己编译要用，链接我的人也要用。
#     比如这里的 include/ 目录，用户用 mathlib 时
#     需要 #include "mathlib/math_tools.h"，所以得传出去。
#
#  2. PRIVATE 的头文件目录
#     就是库内部用的头文件目录，对外不暴露。
#     只有我自己编译的时候需要，链接我的人不需要。
#     比如这里的 src/ 目录，internal_utils.h 是内部用的，
#     不应该让用户看到。
#
#  为什么要分这么细？
#    - 封装性好：用户只看到应该看到的接口
#    - 避免冲突：内部头文件不会污染用户的命名空间
#    - 依赖清晰：谁需要什么，明明白白
#
target_include_directories(mathlib
    PUBLIC
        # PUBLIC：我用，你链接我你也能用
        # $<BUILD_INTERFACE:...> 是 generator expression
        # 意思是：构建的时候用这个路径
        # $<INSTALL_INTERFACE:...> 是安装的时候用的路径
        # 这个咱后面讲 install 的时候再说
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        # PRIVATE：只有我自己用，跟你没关系
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================
#  Generator Expressions（生成器表达式）简介
# ============================================================
#
#  徒弟你看到上面的 $<...> 了吧？这叫生成器表达式。
#
#  啥意思呢？CMake 的执行过程分两个阶段：
#    1. 配置阶段（configure time）：处理 CMakeLists.txt，
#       生成构建系统。这时候普通变量就展开了。
#    2. 构建阶段（build time）：真正编译的时候。
#       这时候生成器表达式才展开。
#
#  为什么需要生成器表达式？
#    因为有些东西配置阶段不知道，得到构建阶段才知道。
#    比如：当前是 Debug 还是 Release？编译的是哪个配置？
#
#  常见的生成器表达式：
#
#    $<CONFIG:Debug>            —— 如果是 Debug 构建，值为 1，否则 0
#    $<IF:条件,真值,假值>        —— 三目运算
#    $<BUILD_INTERFACE:路径>     —— 构建时使用的路径
#    $<INSTALL_INTERFACE:路径>   —— 安装时使用的路径
#    $<TARGET_PROPERTY:目标,属性> —— 获取目标的属性
#
#  举个例子：Debug 模式下加个宏
#    target_compile_definitions(mathlib PRIVATE
#        $<$<CONFIG:Debug>:DEBUG_MODE>
#    )
#  意思是：如果 CONFIG 是 Debug，就加上 DEBUG_MODE 这个宏。
#
#  语法有点怪，$<$<CONFIG:Debug>:xxx>，层层嵌套的尖括号。
#  刚开始看可能觉得别扭，用多了就习惯了。
#
#  生成器表达式很强大，能玩出很多花样。
#  先记住 BUILD_INTERFACE 和 INSTALL_INTERFACE，
#  还有 $<$<CONFIG:Debug>:...> 这种条件表达式就够用了。
#

# ============================================================
#  target_compile_features - 指定 C++ 标准
# ============================================================
#
#  告诉 CMake 这个目标需要哪些 C++ 特性。
#  CMake 会自动给编译器加上对应的 -std=c++xx 选项。
#
#  cxx_std_17 表示需要 C++17 标准。
#  其他可选的：cxx_std_11, cxx_std_14, cxx_std_17, cxx_std_20 等。
#
#  为什么用这个而不是直接 set(CMAKE_CXX_STANDARD 17)？
#    因为 target_compile_features 是针对目标的，
#    不同的目标可以有不同的 C++ 标准。
#    而且它是 PUBLIC/PRIVATE 可传递的——
#    如果一个库 PUBLIC 地要求 C++17，
#    那链接它的可执行文件也自动用 C++17。
#
target_compile_features(mathlib PUBLIC cxx_std_17)


# ============================================================
#  target_compile_options - 编译选项
# ============================================================
#
#  给目标加编译选项，也是分 PUBLIC/PRIVATE 的。
#
#  这里我们用生成器表达式，根据编译器类型加不同的警告选项。
#  $<CXX_COMPILER_ID:GNU,Clang,AppleClang> 表示如果编译器是
#  GCC、Clang 或 AppleClang 中的一种，表达式值为 1。
#
#  注意：警告选项一般用 PRIVATE，因为这是我库自己的编译选项，
#  不需要强加给用我的人。每个人的警告策略可能不一样。
#
target_compile_options(mathlib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# ============================================================
#  target_compile_definitions - 宏定义
# ============================================================
#
#  给目标加宏定义。同样可以用生成器表达式。
#
#  比如 Debug 模式下加个 DEBUG 宏：
#
target_compile_definitions(mathlib PRIVATE
    $<$<CONFIG:Debug>:MATHLIB_DEBUG>
)

# ============================================================
#  导出目标配置（给 find_package 用的）
# ============================================================
#
#  这块稍微高级一点，先了解一下就行。
#
#  如果我们想让别的项目能用 find_package(mathlib) 找到我们的库，
#  就需要导出一些配置文件。
#
#  CMake 提供了 install(EXPORT ...) 来干这个事。
#  咱这里先不演示 install，先让你有个概念。
#
#  简单说就是：把目标的信息（头文件路径、编译选项、依赖的库等）
#  打包成 CMake 文件，别人 find_package 的时候就读这些文件，
#  自动把依赖配置好。
#
#  这就是现代 CMake 的威力——库不只是一堆二进制文件，
#  还带着"怎么使用我"的完整信息。
#

# 给目标起个别名，让别人用 mathlib::mathlib 这种命名空间式的名字引用
# 这是现代 CMake 的惯例，跟 find_package 出来的目标风格一致
add_library(mathlib::mathlib ALIAS mathlib)

# ALIAS 目标就是个别名，不创建新目标，只是给现有目标起个外号。
# 好处：
#   1. 跟 find_package 出来的目标命名风格一致（都是 xxx::xxx）
#   2. 子目录里用和安装后用，名字一样，代码不用改


# ============================================================
#  老师傅的话：
#  这个 CMakeLists.txt 虽短，但全是现代 CMake 的精华。
#
#  记住几个核心思想：
#
#  1. 一切基于目标（target）
#     每个库、每个可执行文件都是一个目标。
#     所有属性（头文件路径、编译选项、依赖库...）都挂在目标上。
#
#  2. PUBLIC / PRIVATE / INTERFACE
#     区分"自己用"还是"别人也要用"。
#     依赖关系自动传递，不用手动管。
#
#  3. 不用全局变量
#     尽量不用 include_directories、add_definitions、
#     link_libraries 这些全局命令。
#     用 target_* 系列，精准控制。
#
#  下一讲咱看一个完整项目怎么组织，把这些知识串起来。
# ============================================================
