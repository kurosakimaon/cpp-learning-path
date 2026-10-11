# ============================================================
#  CMake 示例 05 - 现代 CMake（基于目标）
#  根目录 CMakeLists.txt
# ============================================================

cmake_minimum_required(VERSION 3.15)

project(ModernCMakeDemo VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准的全局默认值（如果子目标不自己指定的话用这个）
# 注意：现代 CMake 推荐在目标上用 target_compile_features，
# 而不是全局设置。这里只是演示一下全局设置的写法。
#
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)  # 不用 GNU 扩展，用纯标准 C++

# 添加子目录
add_subdirectory(mathlib)
add_subdirectory(app)
