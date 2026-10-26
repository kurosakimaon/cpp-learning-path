// ============================================================
//  main.cpp - 选项与配置示例
// ============================================================

#include <iostream>
#include <string>

// 包含由 CMake 生成的配置头文件
#include "config.h"

// 一个简单的函数，用来演示 Debug/Release 的区别
int calculate(int a, int b) {
    // 如果定义了 ENABLE_DEBUG_OUTPUT，就打印调试信息
#ifdef ENABLE_DEBUG_OUTPUT
    std::cout << "  [DEBUG] 计算 " << a << " + " << b << std::endl;
#endif

    return a + b;
}

// 演示实验性功能
void experimentalFeature() {
#ifdef ENABLE_EXPERIMENTAL_FEATURES
    std::cout << "  [实验功能] 这是一个实验性功能！" << std::endl;
    std::cout << "  [实验功能] 正在运行高级算法..." << std::endl;
    std::cout << "  [实验功能] 完成！(假装的)" << std::endl;
#else
    std::cout << "  实验性功能未启用。" << std::endl;
    std::cout << "  想试试？用 -DENABLE_EXPERIMENTAL_FEATURES=ON 重新 cmake。" << std::endl;
#endif
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CMake 选项与配置示例" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // ---- 显示从 config.h 里读出来的信息 ----
    std::cout << "【项目信息】" << std::endl;
    std::cout << "  项目名称：" << PROJECT_NAME << std::endl;
    std::cout << "  项目版本：" << PROJECT_VERSION << std::endl;
    std::cout << "  构建类型：" << BUILD_TYPE << std::endl;
    std::cout << "  作者：" << AUTHOR_NAME << std::endl;
    std::cout << std::endl;

    // ---- 显示哪些功能被启用了 ----
    std::cout << "【功能开关状态】" << std::endl;

#ifdef ENABLE_DEBUG_OUTPUT
    std::cout << "  调试输出：已启用" << std::endl;
#else
    std::cout << "  调试输出：未启用" << std::endl;
#endif

#ifdef ENABLE_EXPERIMENTAL_FEATURES
    std::cout << "  实验功能：已启用" << std::endl;
#else
    std::cout << "  实验功能：未启用" << std::endl;
#endif

    // 这个宏由 target_compile_definitions 直接传入，不经过 config.h
#ifdef CMAKE_DEFINITION_DEMO
    std::cout << "  target_compile_definitions 生效：CMAKE_DEFINITION_DEMO="
              << CMAKE_DEFINITION_DEMO << std::endl;
#endif

    std::cout << std::endl;

    // ---- 测试计算函数 ----
    std::cout << "【计算测试】" << std::endl;
    int result = calculate(10, 20);
    std::cout << "  计算结果：10 + 20 = " << result << std::endl;
    std::cout << std::endl;

    // ---- 测试实验功能 ----
    std::cout << "【实验功能】" << std::endl;
    experimentalFeature();
    std::cout << std::endl;

    // ---- Debug/Release 模式提示 ----
    std::cout << "【构建模式】" << std::endl;
#ifdef NDEBUG
    std::cout << "  当前是 Release 模式（定义了 NDEBUG）" << std::endl;
#else
    std::cout << "  当前是 Debug 模式（未定义 NDEBUG）" << std::endl;
    std::cout << "  断言(assert)是生效的" << std::endl;
#endif
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "  试试用不同的选项重新 cmake 吧！" << std::endl;
    std::cout << "  比如：cmake .. -DENABLE_DEBUG_OUTPUT=ON" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
