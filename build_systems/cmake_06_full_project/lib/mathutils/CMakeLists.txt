# ============================================================
#  lib/mathutils/CMakeLists.txt
#  项目内部的计算器库
# ============================================================

# 不写 STATIC/SHARED，由全局 BUILD_SHARED_LIBS 选项决定
add_library(mathutils
    src/calculator.cpp
)

# Windows 下 DLL 默认不导出符号。开启自动导出后，
# BUILD_SHARED_LIBS=ON 时 MSVC/MinGW 都能正常链接。
# 真实项目通常改用显式导出宏或 generate_export_header。
if(WIN32)
    set_target_properties(mathutils PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# 命名空间风格的别名，使用方式和 find_package 导出的目标一致
add_library(myproject::mathutils ALIAS mathutils)

target_include_directories(mathutils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# PUBLIC：链接 mathutils 的目标自动获得 C++17 设置
target_compile_features(mathutils PUBLIC cxx_std_17)

# 安装公共头文件
install(DIRECTORY include/
    DESTINATION include
)
