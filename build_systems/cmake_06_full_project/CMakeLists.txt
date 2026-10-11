# ============================================================
#  CMake 示例 06 - 完整项目结构
#  根目录 CMakeLists.txt
#  老师傅注释版
# ============================================================

# ---- 最低 CMake 版本 ----
# 3.15 是个比较合理的最低版本，主流 Linux 发行版都有了
cmake_minimum_required(VERSION 3.15)

# ============================================================
#  项目设置
# ============================================================
#
# project 命令可以一次性设置项目名、版本、语言。
# VERSION 后面跟版本号（主.次.补丁.tweak）
# LANGUAGES 指定使用的语言，CXX 是 C++
#
# 设了 VERSION 之后，会自动定义这些变量：
#   PROJECT_VERSION           = 2.1.0
#   PROJECT_VERSION_MAJOR     = 2
#   PROJECT_VERSION_MINOR     = 1
#   PROJECT_VERSION_PATCH     = 0
#   PROJECT_VERSION_TWEAK     = 0（没写就是 0）
#
project(MyProject
    VERSION 2.1.0
    DESCRIPTION "一个完整的 CMake 示例项目"
    LANGUAGES CXX
)

# ============================================================
#  全局设置（尽量少用，能用 target 就用 target）
# ============================================================

# C++ 标准的全局默认设置
# 注意：现代 CMake 推荐在目标上用 target_compile_features，
# 但设置一个全局默认值也没坏处，省得每个目标都写一遍。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 必须支持 C++17，不支持就报错
set(CMAKE_CXX_EXTENSIONS OFF)         # 禁用 GNU 扩展，用纯标准 C++

# 构建目录的位置——其实不用设，外部构建的时候自然在 build 里
# 这里只是提一下有这些变量

# 默认构建类型（如果用户没指定的话）
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "没有指定构建类型，默认使用 Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)
    # 设置可选值，cmake-gui 里会有下拉框
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()


# ============================================================
#  CMake 模块路径
# ============================================================
#
#  把我们自己的 cmake/ 目录加到 CMAKE_MODULE_PATH 里，
#  这样 include(xxx) 的时候 CMake 会去这个目录里找。
#
#  CMAKE_MODULE_PATH 是个列表，用 list(APPEND ...) 追加，
#  别直接 set 覆盖了，不然系统模块可能找不到。
#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# ============================================================
#  选项（Options）
# ============================================================

option(BUILD_TESTS "构建单元测试" ON)
option(BUILD_SHARED_LIBS "构建共享库（动态库）" OFF)
option(WARNINGS_AS_ERRORS "把警告当作错误" OFF)

message(STATUS "===== 项目配置 =====")
message(STATUS "项目名称：${PROJECT_NAME}")
message(STATUS "项目版本：${PROJECT_VERSION}")
message(STATUS "构建类型：${CMAKE_BUILD_TYPE}")
message(STATUS "构建测试：${BUILD_TESTS}")
message(STATUS "共享库：${BUILD_SHARED_LIBS}")
message(STATUS "警告当错误：${WARNINGS_AS_ERRORS}")
message(STATUS "====================")


# ============================================================
#  生成配置头文件
# ============================================================
#
#  把版本号等信息写到 version.h 里，让 C++ 代码也能拿到。
#  模板文件在 include/myproject/version.h.in
#  生成的文件在构建目录下的 include/myproject/version.h
#
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/myproject/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/myproject/version.h"
    @ONLY   # 只替换 @VAR@，不替换 ${VAR}，更安全
)


# ============================================================
#  查找第三方库
# ============================================================
#
#  find_package 是 CMake 里找第三方库的标准方式。
#
#  它会去找 FindXXX.cmake 模块或者 XXXConfig.cmake 配置文件，
#  找到了就会定义一些目标（比如 Threads::Threads），
#  你直接链接就行，不用管头文件在哪、库文件在哪。
#
#  Threads 是 CMake 自带的模块，用来找系统的线程库。
#  Linux 下一般是 pthread。
#
#  REQUIRED 表示这个库是必须的，找不到就报错。
#
find_package(Threads REQUIRED)

# 找完之后，你可以链接 Threads::Threads 这个目标。
# 它会自动加上正确的头文件路径和库。


# ============================================================
#  子目录
# ============================================================

# 库
add_subdirectory(lib/mathutils)

# 主程序
# 我们直接在这里定义可执行文件目标，不用再套一层 app 目录了
# （当然你要套也可以，看个人喜好）

add_executable(myproject_app
    src/main.cpp
    src/app/app.cpp
)

# 起个别名，跟命名空间风格一致
add_executable(myproject::app ALIAS myproject_app)

# 头文件路径
target_include_directories(myproject_app
    PRIVATE
        src                          # 自己内部用的头文件
        ${CMAKE_CURRENT_BINARY_DIR}/include  # 生成的 version.h
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 链接库
# - mathutils 是我们自己的库
# - Threads::Threads 是 find_package 找出来的系统线程库
target_link_libraries(myproject_app
    PRIVATE
        myproject::mathutils
        Threads::Threads
)

# 编译宏定义
# 把构建类型传给 C++ 代码
target_compile_definitions(myproject_app
    PRIVATE
        BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

# 编译警告——用我们自定义的模块
include(CompilerWarnings)
set_project_warnings(myproject_app)


# ============================================================
#  测试
# ============================================================
#
#  enable_testing() 打开测试功能。
#  必须在根 CMakeLists.txt 里调用，而且要在 add_test 之前。
#  打开之后，就可以用 ctest 命令运行测试了。
#
#  我们用一个选项控制要不要构建测试，用户可以选择关掉。
#
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "已启用单元测试")
endif()


# ============================================================
#  安装规则（Install）
# ============================================================
#
#  install 命令用来定义"make install"（或 cmake --install）
#  要安装什么、安装到哪。
#
#  常用的 install 子命令：
#    install(TARGETS ...)    —— 安装目标（可执行文件、库）
#    install(FILES ...)      —— 安装文件（头文件、配置文件等）
#    install(DIRECTORY ...)  —— 安装目录
#    install(EXPORT ...)     —— 导出版本信息（给 find_package 用）
#
#  安装的根目录由 CMAKE_INSTALL_PREFIX 控制，
#  默认 Linux 下是 /usr/local，Windows 下是 C:\Program Files\...
#  可以通过 cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install 修改。
#

# 安装可执行文件
install(TARGETS myproject_app
    RUNTIME DESTINATION bin        # 可执行文件装到 bin/ 目录
    LIBRARY DESTINATION lib        # 动态库装到 lib/
    ARCHIVE DESTINATION lib        # 静态库装到 lib/
)

# 安装 mathutils 库
install(TARGETS mathutils
    EXPORT myproject-targets       # 导出版本信息
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 安装生成的 version.h
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/myproject/version.h"
    DESTINATION include/myproject
)

# 导出目标配置（让别的项目能用 find_package 找到我们）
install(EXPORT myproject-targets
    FILE myproject-targets.cmake
    NAMESPACE myproject::
    DESTINATION lib/cmake/myproject
)

# 【老师傅补充】
# 真正要做 find_package 支持，还需要写一个 myproject-config.cmake
# 以及版本配置文件。那属于比较高级的内容了，咱这个例子就点到为止。
# 有兴趣自己去查查 CMake 官方文档里的 "installing exports"。


# ============================================================
#  CPack 打包
# ============================================================
#
#  CPack 是 CMake 自带的打包工具，能把项目打包成
#  .tar.gz、.zip、.deb、.rpm、NSIS 安装包等等。
#
#  简单用的话，就下面这几行。
#  然后运行 cpack 就能生成安装包了。
#

set(CPACK_PACKAGE_NAME "MyProject")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "一个完整的 CMake 示例项目")
set(CPACK_PACKAGE_VENDOR "老师傅和徒弟")

# 要打什么格式的包，自己选
# 常用的：ZIP（zip压缩包）、TGZ（tar.gz）、DEB（deb包）、RPM（rpm包）、NSIS（Windows安装包）
set(CPACK_GENERATOR "ZIP;TGZ")

# 这行必须放在最后，include 了 CPack 才行
include(CPack)


# ============================================================
#  老师傅的话：
#
#  恭喜你！看到这里，说明你已经把这 6 个例子都过了一遍。
#  这个示例 6 就是一个比较规范的 C++ 项目的 CMake 写法了。
#
#  回顾一下我们学了啥：
#    1. 基础：cmake_minimum_required / project / add_executable
#    2. 多文件：变量 / include_directories / file(GLOB)的坑
#    3. 库：add_library / target_link_libraries / 静态库vs动态库
#    4. 配置：option / configure_file / CMAKE_BUILD_TYPE
#    5. 现代CMake：target_* / PUBLIC-PRIVATE-INTERFACE / generator expressions
#    6. 完整项目：find_package / 测试 / 安装 / 打包
#
#  CMake 的东西很多，不用一下子全记住。
#  先用起来，遇到问题再查，慢慢就熟练了。
#
#  祝你 CMake 用得愉快！
# ============================================================
