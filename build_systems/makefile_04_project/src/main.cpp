/*
 * 示例4：主程序入口
 * 完整项目结构的主函数
 */
#include <iostream>
#include "application.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    LOG_INFO("程序启动");
    LOG_INFO("参数个数: " + std::to_string(argc));

    app::Application app;

    // 初始化
    if (!app.init()) {
        LOG_ERROR("初始化失败！");
        return 1;
    }

    // 运行
    int ret = app.run();

    // 清理
    app.shutdown();

    LOG_INFO("程序退出，返回码: " + std::to_string(ret));
    return ret;
}
