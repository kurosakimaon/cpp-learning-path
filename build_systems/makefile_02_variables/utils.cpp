/*
 * 示例2：工具函数实现
 * 徒弟你看，把功能分散到不同文件里，这才是真实项目的样子
 */
#include <iostream>
#include "utils.h"

void print_welcome(const char* name) {
    std::cout << "==================================" << std::endl;
    std::cout << "  欢迎你，" << name << "！" << std::endl;
    std::cout << "  这是第2个Makefile示例" << std::endl;
    std::cout << "==================================" << std::endl;
}

int add(int a, int b) {
    return a + b;
}
