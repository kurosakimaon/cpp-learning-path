# ================================================================
#  CMakeLists.txt —— Todo CLI 项目构建配置
# ================================================================
#
#  徒弟啊，这是 CMake 的构建配置文件。
#  用 CMake 的好处是跨平台，不管 Windows、Linux 还是 Mac，
#  都能用同一套配置生成对应的构建文件。
#
#  这个项目的结构：
#    - include/  头文件
#    - src/      源文件
#    - tests/    测试代码
#
#  我们生成两个可执行文件：
#    - todo_cli        主程序
#    - test_task       单元测试
#

# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(TodoCLI
    VERSION 1.0
    DESCRIPTION "一个简单的命令行待办事项工具"
    LANGUAGES CXX
)

# C++ 标准：用 C++17
# 为什么选 C++17？因为它足够新，有很多好用的特性，
# 而且主流编译器都支持了。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 必须支持 C++17
set(CMAKE_CXX_EXTENSIONS OFF)         # 不用编译器扩展，保持标准

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ================================================================
#  主程序
# ================================================================

# 源文件列表
set(SOURCES
    src/Task.cpp
    src/TaskList.cpp
    src/FileStorage.cpp
    src/Command.cpp
    src/CommandFactory.cpp
    src/ConsoleUI.cpp
    src/Utils.cpp
    src/main.cpp
)

# 生成可执行文件
add_executable(todo_cli ${SOURCES})

# 头文件目录
target_include_directories(todo_cli PRIVATE include)

# 编译选项（根据编译器不同设置不同的警告）
if(MSVC)
    # Visual Studio 的警告
    target_compile_options(todo_cli PRIVATE /W4)
else()
    # GCC/Clang 的警告
    target_compile_options(todo_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ================================================================
#  测试程序
# ================================================================

add_executable(test_task
    tests/test_task.cpp
    src/Task.cpp
    src/TaskList.cpp
)

target_include_directories(test_task PRIVATE include)

if(MSVC)
    target_compile_options(test_task PRIVATE /W4)
else()
    target_compile_options(test_task PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ================================================================
#  启用测试（可选）
# ================================================================
#  如果你想使用 CTest，可以打开下面这行
# enable_testing()
# add_test(NAME TaskTest COMMAND test_task)

# ================================================================
#  编译方法
# ================================================================
#
#  Linux / macOS:
#    mkdir build
#    cd build
#    cmake ..
#    make
#    ./bin/todo_cli
#
#  Windows (Visual Studio):
#    mkdir build
#    cd build
#    cmake ..
#    cmake --build . --config Release
#    Release\todo_cli.exe
#
#  Windows (MinGW):
#    mkdir build
#    cd build
#    cmake .. -G "MinGW Makefiles"
#    mingw32-make
#    bin\todo_cli.exe
#
