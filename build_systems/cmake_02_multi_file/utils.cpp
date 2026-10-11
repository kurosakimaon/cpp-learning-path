// ============================================================
//  utils.cpp - 工具函数实现
// ============================================================

#include "utils.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

// 打印一个 40 个字符长的分隔线
void printSeparator() {
    std::cout << "========================================" << std::endl;
}

// 打印带框的标题，看起来花哨一点
void printTitle(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << std::endl;
    printSeparator();
}

// 获取当前时间的字符串
std::string getCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
