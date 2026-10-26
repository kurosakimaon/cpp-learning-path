// ================================================================
//  FileStorage.h —— 文件存储实现
//  把任务保存到文本文件，从文本文件加载
// ================================================================
//
//  这是 Storage 接口的具体实现：用文件来存数据。
//
//  文件格式很简单：每行一个任务，用竖线分隔各字段。
//  为什么不用 JSON？
//  因为 JSON 需要第三方库，为了保持项目零依赖，
//  我们用简单的文本格式。
//  以后想换成 JSON 也很简单，只要再写个 JsonStorage 就行。
//
//  这里体现了 RAII 思想吗？
//  其实 C++ 的 fstream 本身就是 RAII 的——
//  构造时打开文件，析构时自动关闭。
//  所以你不用手动调用 close()，出了作用域自动关。
//

#ifndef TODO_CLI_FILESTORAGE_H
#define TODO_CLI_FILESTORAGE_H

#include "Storage.h"
#include <string>

namespace todo {

class FileStorage : public Storage {
public:
    // 构造时指定文件名
    explicit FileStorage(std::string filename);

    // 保存任务到文件
    bool save(const std::vector<Task>& tasks) override;

    // 从文件加载任务
    bool load(std::vector<Task>& out_tasks) override;

    // 文件是否存在且可读
    bool exists() const override;

    // 获取文件名
    const std::string& getFilename() const { return filename_; }

private:
    std::string filename_;  // 数据文件路径
};

} // namespace todo

#endif // TODO_CLI_FILESTORAGE_H
