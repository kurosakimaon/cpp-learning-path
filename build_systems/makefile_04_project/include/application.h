/*
 * 示例4：应用程序类头文件
 * 高层业务逻辑
 */
#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include "calculator.h"

namespace app {

class Application {
public:
    Application();
    ~Application();

    // 初始化
    bool init();

    // 运行
    int run();

    // 清理
    void shutdown();

private:
    math::Calculator calc_;

    // 演示计算
    void demo_calculator();

    // 演示字符串工具
    void demo_string_utils();
};

} // namespace app

#endif // APPLICATION_H
