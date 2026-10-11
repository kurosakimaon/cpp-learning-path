/*
 * 示例2：主程序
 * 调用 utils.cpp 里的函数
 */
#include <iostream>
#include "utils.h"

int main() {
    print_welcome("徒弟");

    int result = add(3, 5);
    std::cout << "3 + 5 = " << result << std::endl;

    return 0;
}
