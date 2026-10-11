// ============================================================
//  app.h - 主应用类
// ============================================================

#ifndef MYPROJECT_APP_H
#define MYPROJECT_APP_H

#include <string>
#include <vector>

namespace myproject {

// 应用程序类
class App {
public:
    App();
    ~App();

    // 运行应用，返回退出码
    int run(int argc, char* argv[]);

    // 设置欢迎消息
    void setWelcomeMessage(const std::string& msg);

private:
    std::string welcome_message_;

    // 打印帮助信息
    void printHelp();

    // 打印版本信息
    void printVersion();

    // 处理命令行参数
    bool parseArgs(int argc, char* argv[]);
};

} // namespace myproject

#endif // MYPROJECT_APP_H
