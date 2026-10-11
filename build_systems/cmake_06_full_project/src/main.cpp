// ============================================================
//  main.cpp - 程序入口
// ============================================================

#include "app/app.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        myproject::App app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误！" << std::endl;
        return 1;
    }
}
