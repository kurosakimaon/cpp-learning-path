# ============================================================
#  app 子目录的 CMakeLists.txt
#  构建使用数学库的应用程序
# ============================================================

# 先创建可执行文件
add_executable(math_app main.cpp)

# ============================================================
#  target_link_libraries：链接库
# ============================================================
#
# 格式：target_link_libraries(目标名  <PRIVATE|PUBLIC|INTERFACE>  库名...)
#
# 这就是把库链接到可执行文件上的命令。
# mathlib 是我们在 mathlib/ 目录里定义的库目标。
#
# 注意：因为 mathlib 已经用 target_include_directories(... PUBLIC ...)
# 把自己的头文件路径"公布"出去了，所以这里链接 mathlib 之后，
# math_app 自动就能找到 math_functions.h 了，不用再单独加
# include_directories。
#
# 这就是现代 CMake 的好处——依赖关系自动传递。
#
target_link_libraries(math_app PRIVATE mathlib)

# 这里用 PRIVATE，因为 math_app 是个可执行文件，
# 不会再有别的东西链接它了，所以用 PRIVATE 就够了。
# 如果 math_app 本身也是个库，那就要考虑用 PUBLIC 还是 PRIVATE。
