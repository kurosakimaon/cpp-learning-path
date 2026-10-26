#include "application.h"

#include <exception>
#include <iostream>

int main() {
    try {
        demo::Application app;
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "程序出错：" << e.what() << std::endl;
        return 1;
    }
}
