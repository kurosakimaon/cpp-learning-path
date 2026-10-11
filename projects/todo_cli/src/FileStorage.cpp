// ================================================================
//  FileStorage.cpp —— 文件存储实现
// ================================================================

#include "FileStorage.h"
#include <fstream>
#include <iostream>

namespace todo {

// ================================================================
//  构造函数
// ================================================================

FileStorage::FileStorage(std::string filename)
    : filename_(std::move(filename)) {
    // 这里用 std::move 转移 string 的所有权，避免一次拷贝
    // （小优化，体现移动语义的应用）
}

// ================================================================
//  保存任务到文件
// ================================================================

bool FileStorage::save(const std::vector<Task>& tasks) {
    // 打开文件用于写入
    // ofstream 是 RAII 的：构造时打开，析构时自动关闭
    std::ofstream ofs(filename_);

    if (!ofs.is_open()) {
        std::cerr << "错误：无法打开文件 " << filename_ << " 进行写入" << std::endl;
        return false;
    }

    // 逐行写入，每行一个任务
    for (const auto& task : tasks) {
        ofs << task.serialize() << std::endl;
    }

    // ofs 在这里析构，自动关闭文件
    // 不用手动调用 close()
    return true;
}

// ================================================================
//  从文件加载任务
// ================================================================

bool FileStorage::load(std::vector<Task>& out_tasks) {
    // 打开文件用于读取
    std::ifstream ifs(filename_);

    if (!ifs.is_open()) {
        // 文件不存在不算错误，可能是第一次运行
        // 返回 true 但列表为空
        return true;
    }

    out_tasks.clear();

    std::string line;
    while (std::getline(ifs, line)) {
        // 跳过空行
        if (line.empty()) continue;

        Task task;
        if (Task::deserialize(line, task)) {
            out_tasks.push_back(task);
        } else {
            std::cerr << "警告：跳过格式错误的行：" << line << std::endl;
        }
    }

    // ifs 在这里析构，自动关闭文件
    return true;
}

} // namespace todo
