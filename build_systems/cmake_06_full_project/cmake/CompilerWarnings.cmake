# ============================================================
#  CompilerWarnings.cmake
#  自定义 CMake 模块：设置编译警告选项
# ============================================================
#
#  把常用的编译警告设置封装成一个函数，
#  各个目标想用的时候调用一下就行。
#  这样不用每个地方都写一大段 if/else。
#
#  这就是自定义 CMake 模块的好处——复用代码。
#
#  怎么用？
#    首先在根 CMakeLists.txt 里把 cmake/ 目录加到模块搜索路径：
#      list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#
#    然后在需要的地方 include 进来：
#      include(CompilerWarnings)
#
#    最后调用函数：
#      set_project_warnings(my_target)
#
# ============================================================

# 定义一个函数，给指定目标设置项目标准的警告选项
# 函数名：set_project_warnings
# 参数：target_name —— 目标名
function(set_project_warnings target_name)

    # 选项：要不要把警告当错误
    # 默认 OFF，你可以在 cmake 时用 -DWARNINGS_AS_ERRORS=ON 打开
    option(WARNINGS_AS_ERRORS "把编译警告当作错误" OFF)

    set(MSVC_WARNINGS
        /W4         # 警告等级 4
        /permissive-  # 标准模式，禁用 MSVC 扩展
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow               # 变量遮蔽警告
        -Wnon-virtual-dtor     # 有虚函数的类没有虚析构函数
        -Wold-style-cast       # C 风格类型转换
        -Wcast-align           # 类型转换导致对齐问题
        -Wunused               # 未使用的东西
        -Wconversion           # 隐式类型转换可能丢精度
        -Wsign-conversion      # 有符号/无符号转换
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}      # GCC 跟 Clang 差不多
        -Wmisleading-indentation   # 缩进误导
        -Wduplicated-cond          # 重复条件
        -Wduplicated-branches      # 重复分支
        -Wlogical-op               # 逻辑操作符问题
        -Wuseless-cast             # 无用的类型转换
    )

    # 如果警告当错误，加上对应的选项
    if(WARNINGS_AS_ERRORS)
        message(STATUS "警告将被当作错误处理")
        list(APPEND MSVC_WARNINGS /WX)
        list(APPEND CLANG_WARNINGS -Werror)
        list(APPEND GCC_WARNINGS -Werror)
    endif()

    # 根据编译器类型设置对应的警告选项
    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target_name} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "未识别的编译器，没有设置警告选项")
    endif()

endfunction()
