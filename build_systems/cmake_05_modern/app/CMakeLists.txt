# ============================================================
#  app 子目录 CMakeLists.txt
# ============================================================

add_executable(stats_app main.cpp)

# 链接 mathlib 库
# 注意：我们用了 mathlib::mathlib 这个别名目标
# 这是现代 CMake 的推荐写法，跟 find_package 风格一致
#
# 因为 mathlib 已经 PUBLIC 地设置了：
#   - include 目录（target_include_directories PUBLIC）
#   - C++17 标准（target_compile_features PUBLIC）
# 所以 stats_app 自动就能继承这些设置，不用再手动加了。
#
# 这就是基于目标的 CMake 的好处——依赖自动传递。
#
target_link_libraries(stats_app PRIVATE mathlib::mathlib)

# 给 app 也加一些自己的编译选项
target_compile_options(stats_app PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
