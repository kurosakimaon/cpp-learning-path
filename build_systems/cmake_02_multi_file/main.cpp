// ============================================================
//  main.cpp - 主程序
// ============================================================
//  徒弟你看，这个 main.cpp 里用到了 utils 和 math 两个模块的函数。
//  编译的时候得把 utils.cpp 和 math.cpp 也一起编进去，
//  不然链接的时候会报 "undefined reference" 错误。
// ============================================================

#include <iostream>
#include "utils.h"   // 包含工具函数头文件
#include "math.h"    // 包含数学函数头文件

int main() {
    // 用我们写的工具函数打印标题
    printTitle("CMake 多文件示例演示");

    std::cout << "当前时间：" << getCurrentTimeString() << std::endl;
    std::cout << std::endl;

    // ---- 测试阶乘 ----
    std::cout << "【阶乘测试】" << std::endl;
    for (int i = 0; i <= 10; ++i) {
        std::cout << "  " << i << "! = " << factorial(i) << std::endl;
    }
    std::cout << std::endl;

    // ---- 测试斐波那契数列 ----
    std::cout << "【斐波那契数列】" << std::endl;
    std::cout << "  前 15 项：";
    for (int i = 0; i < 15; ++i) {
        std::cout << fibonacci(i) << " ";
    }
    std::cout << std::endl << std::endl;

    // ---- 测试质数判断 ----
    std::cout << "【质数判断】" << std::endl;
    std::cout << "  100 以内的质数：" << std::endl << "  ";
    int count = 0;
    for (int i = 2; i <= 100; ++i) {
        if (isPrime(i)) {
            std::cout << i << " ";
            count++;
            if (count % 10 == 0) std::cout << std::endl << "  ";
        }
    }
    std::cout << std::endl;

    // ---- 测试最大公约数 ----
    std::cout << "【最大公约数】" << std::endl;
    std::cout << "  gcd(48, 18) = " << gcd(48, 18) << std::endl;
    std::cout << "  gcd(100, 25) = " << gcd(100, 25) << std::endl;
    std::cout << "  gcd(7, 13) = " << gcd(7, 13) << std::endl;
    std::cout << std::endl;

    printSeparator();
    std::cout << "  演示完毕，程序正常结束！" << std::endl;
    printSeparator();

    return 0;
}
