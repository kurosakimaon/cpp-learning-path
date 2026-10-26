# ============================================================
#  CMake 示例 04 - 选项与配置
#  老师傅注释版
# ============================================================

# 最低版本要求
cmake_minimum_required(VERSION 3.10)

# ============================================================
#  project 还能指定版本号
# ============================================================
#
# project 命令不只是起名字，还能顺便设置版本。
# VERSION 后面跟版本号（格式：主版本.次版本.补丁版本. tweak）
# 设了之后，CMake 会自动定义这些变量：
#   PROJECT_VERSION          → 1.2.3
#   PROJECT_VERSION_MAJOR    → 1
#   PROJECT_VERSION_MINOR    → 2
#   PROJECT_VERSION_PATCH    → 3
#
# LANGUAGES 指定项目用什么语言，CXX 就是 C++。
# 不写的话默认是 C 和 C++ 都启用。
#
project(OptionsDemo VERSION 1.2.3 LANGUAGES CXX)

# ============================================================
#  自定义变量
# ============================================================
#
# 用 set 定义一个变量，后面 configure_file 的时候能用
# 这个变量的值会被填到 config.h.in 里的 @AUTHOR_NAME@
#
set(AUTHOR_NAME "老师傅和徒弟")


# ============================================================
#  option：定义编译选项开关
# ============================================================
#
# 格式：option(选项名  "描述文字"  默认值)
#
# 这是 CMake 里最常用的功能之一，用来做编译开关。
# 用户在 cmake 的时候可以用 -D选项名=ON/OFF 来控制。
#
# 比如：
#   cmake .. -DENABLE_DEBUG_OUTPUT=ON
#   cmake .. -DENABLE_EXPERIMENTAL_FEATURES=OFF
#
# 注意：option 的值只能是 ON 或 OFF（也就是布尔值）。
# 如果你要传字符串或者数字，别用 option，直接用 set 或者 -D。
#

# 调试输出开关，默认关闭
option(ENABLE_DEBUG_OUTPUT "是否启用调试输出" OFF)

# 实验性功能开关，默认关闭
option(ENABLE_EXPERIMENTAL_FEATURES "是否启用实验性功能" OFF)

# ============================================================
#  CMAKE_BUILD_TYPE：构建类型
# ============================================================
#
# CMake 内置了几种构建类型：
#   Debug      —— 调试版，有调试信息，不优化
#   Release    —— 发布版，优化，无调试信息
#   RelWithDebInfo —— 带调试信息的发布版
#   MinSizeRel —— 最小体积发布版
#
# 怎么设置？cmake 的时候加参数：
#   cmake .. -DCMAKE_BUILD_TYPE=Debug
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#
# 【老师傅提醒】
#  1. 对于单配置生成器（比如 Makefile、Ninja），
#     CMAKE_BUILD_TYPE 才有用。
#  2. 对于多配置生成器（比如 Visual Studio），
#     这个变量没用，得在编译的时候指定 --config Debug/Release。
#  3. 如果不设置，默认是空的，既不是 Debug 也不是 Release，
#     没有优化也没有调试信息。一般我们会给个默认值。
#

# 如果没指定构建类型，默认用 Debug
# 多配置生成器（Visual Studio、Xcode）不用 CMAKE_BUILD_TYPE，
# 所以这里也要排除掉，避免往缓存里塞一个不起作用的假值。
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# CACHE STRING 是什么意思？
# 简单说就是把这个变量放到"缓存"里，用户能在 cmake-gui 里看到并修改。
# FORCE 是强制覆盖缓存里已有的值。
# 刚开始不用太纠结，照抄就行。


# ============================================================
#  configure_file：生成配置头文件
# ============================================================
#
# 这可是个好东西！它能把 .in 模板文件里的变量替换掉，
# 生成一个真正的头文件。
#
# 格式：configure_file(输入文件  输出文件)
#
# 输入文件里的 @VAR@ 和 ${VAR} 会被替换成 CMake 变量 VAR 的值。
# 还有个特殊的 #cmakedefine 宏，会根据变量是否为真，
# 变成 #define 或者 /* #undef VAR */。
#
# 生成的文件会放在构建目录里，所以后面要把构建目录
# 加到 include 路径里。
#
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"   # 输入模板
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"      # 输出文件
)
# CMAKE_CURRENT_BINARY_DIR 是当前的构建目录（也就是 build 目录）
# 生成的 config.h 就放在那里


# ============================================================
#  if / elseif / else：条件判断
# ============================================================
#
# CMake 的条件判断语法跟别的语言不太一样，
# 没有花括号，用 endif 结尾。
#
# 常用的条件：
#   if(VAR)            —— 变量为真（ON/TRUE/1/非空字符串）
#   if(NOT VAR)        —— 变量为假
#   if(VAR1 AND VAR2)  —— 逻辑与
#   if(VAR1 OR VAR2)   —— 逻辑或
#   if(DEFINED VAR)    —— 变量是否定义了
#   if(EXISTS 文件)    —— 文件是否存在
#   if( STREQUAL )     —— 字符串相等比较
#
# 下面我们根据构建类型，加不同的编译选项。
#

message(STATUS "构建类型：${CMAKE_BUILD_TYPE}")
message(STATUS "调试输出：${ENABLE_DEBUG_OUTPUT}")
message(STATUS "实验功能：${ENABLE_EXPERIMENTAL_FEATURES}")
# message 就是打印消息，STATUS 表示这是状态信息，
# 会带两个减号 -- 前缀。


# ============================================================
#  创建可执行文件
# ============================================================
add_executable(options_demo main.cpp)


# ============================================================
#  target_compile_definitions：添加编译宏定义
# ============================================================
#
# 相当于在命令行上加 -DXXX。
# 跟之前的 target_include_directories 一样，
# 也是针对单个目标的，也有 PUBLIC/PRIVATE/INTERFACE。
#
# 注意：ENABLE_DEBUG_OUTPUT / ENABLE_EXPERIMENTAL_FEATURES 已经由
# config.h 的 #cmakedefine 生成。如果这里再用 -D 定义同名宏，
# 命令行会先把它定义成 1，而 config.h 里又 #define 一次空值，
# 编译器会报“宏重定义”。所以这里换一个独立的宏来演示，
# 避免同一个名字被两套机制重复定义。
#
target_compile_definitions(options_demo PRIVATE CMAKE_DEFINITION_DEMO=1)

# 【补充】
# configure_file 和 target_compile_definitions 都能把配置传给代码：
#   - 配置项很多、需要集中管理时，用 configure_file + config.h
#   - 只是给某个目标加一两个宏时，用 target_compile_definitions
# 两种方式都有人用，关键是别对同一个宏重复定义。


# ============================================================
#  target_include_directories：添加头文件路径
# ============================================================
#
# 因为 config.h 是生成在构建目录里的（CMAKE_CURRENT_BINARY_DIR），
# 所以要把构建目录加到搜索路径里，不然 #include "config.h" 找不到。
#
target_include_directories(options_demo PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)


# ============================================================
#  设置编译警告选项
# ============================================================
#
# 不同的编译器，警告选项不一样。
# GCC/Clang 用 -Wall -Wextra，MSVC 用 /W4。
# 所以得判断一下编译器类型。
#

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC 或 Clang
    target_compile_options(options_demo PRIVATE
        -Wall       # 开启大部分警告
        -Wextra     # 额外的警告
        -Wpedantic  # 严格遵循标准
    )
    # Debug 模式下再加个 -g 其实不用额外加，CMake 会自动加
    # Release 模式下加 -O2，CMake 也会自动加
    # 这就是 CMAKE_BUILD_TYPE 的好处——常用选项 CMake 都帮你设好了

elseif(MSVC)
    # Visual Studio
    target_compile_options(options_demo PRIVATE
        /W4     # 警告等级 4
    )
endif()

# 【老师傅经验】
# target_compile_options 也是现代 CMake 的写法。
# 老派写法是直接改 CMAKE_CXX_FLAGS 全局变量，
# 但那样所有目标都受影响，不够灵活。
# 能不用全局就不用全局。


# ============================================================
#  设置 C++ 标准
# ============================================================
#
# 方法一：用 target_compile_features（现代写法，推荐）
# 告诉 CMake 我需要 C++17 的特性。
# 这样 CMake 会自动给编译器加上对应的选项（-std=c++17 或 /std:c++17）
#
# 方法二：设置 CMAKE_CXX_STANDARD 变量（全局的）
#  set(CMAKE_CXX_STANDARD 17)
#  set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# 推荐用 target_compile_features，更精准。
#
target_compile_features(options_demo PRIVATE cxx_std_17)


# ============================================================
#  老师傅的话：
#  这一讲内容比较多，你慢慢消化。
#
#  重点掌握：
#  1. option 定义开关，cmake 时用 -DXXX=ON/OFF 控制
#  2. configure_file 生成 config.h，把 CMake 变量传到 C++ 里
#  3. CMAKE_BUILD_TYPE 控制 Debug/Release
#  4. target_compile_options / target_compile_definitions
#     都是针对单个目标的，比全局变量好
#  5. if/else/endif 做条件判断
#
#  下一讲咱深入讲讲现代 CMake 的基于目标的写法，
#  这可是现在的主流方向。
# ============================================================
