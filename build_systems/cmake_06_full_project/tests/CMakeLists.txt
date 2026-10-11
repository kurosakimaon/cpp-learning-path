# ============================================================
#  tests 子目录 CMakeLists.txt
#  单元测试
# ============================================================

# ---- 测试可执行文件 ----
add_executable(test_calculator test_calculator.cpp)

target_link_libraries(test_calculator
    PRIVATE
        myproject::mathutils
)

# 给测试也加上警告
include(CompilerWarnings)
set_project_warnings(test_calculator)


# ============================================================
#  add_test：注册测试
# ============================================================
#
#  格式：add_test(NAME 测试名 COMMAND 可执行文件 [参数...])
#
#  注册之后，就可以用 ctest 命令来运行测试了。
#  ctest 是 CMake 自带的测试运行工具。
#
#  注意：enable_testing() 要在根 CMakeLists.txt 里调用，
#  而且必须在 add_test 之前调用。
#
add_test(
    NAME CalculatorTest
    COMMAND test_calculator
)

# 如果有多个测试文件，继续 add_executable + add_test 就行
# add_test 注册的测试，返回 0 表示通过，非 0 表示失败
