// ================================================================
//  FileStorage.cpp —— 文件存储实现
// ================================================================

#include "FileStorage.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>
#include <unordered_set>

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
    // 先写临时文件，写完再改名替换原文件。
    // 如果直接打开 todos.txt 写入，写到一半失败（磁盘满、程序崩溃）
    // 会把原文件截断，之前的任务就全丢了。
    const std::string temp_name = filename_ + ".tmp";
    std::ofstream ofs(temp_name, std::ios::binary | std::ios::trunc);

    if (!ofs.is_open()) {
        std::cerr << "错误：无法打开文件 " << temp_name << " 进行写入" << std::endl;
        return false;
    }

    // 逐行写入，每行一个任务
    for (const auto& task : tasks) {
        ofs << task.serialize() << '\n';
    }
    ofs.flush();

    // 写入过程中也可能失败（比如磁盘满），
    // 光检查 is_open() 不够，还要检查流的状态。
    if (!ofs) {
        ofs.close();
        std::error_code ignored;
        std::filesystem::remove(temp_name, ignored);
        std::cerr << "错误：写入文件 " << temp_name << " 失败" << std::endl;
        return false;
    }

    ofs.close();
    if (ofs.fail()) {
        std::error_code ignored;
        std::filesystem::remove(temp_name, ignored);
        std::cerr << "错误：关闭文件 " << temp_name << " 失败" << std::endl;
        return false;
    }

    // 同一目录内的改名通常是原子操作：要么看到旧文件，要么看到完整的新文件，
    // 不会出现只写了一半的 todos.txt。注意这不等于“断电后一定不丢数据”，
    // 真正要求掉电安全还需要刷盘（fsync）。
    std::error_code ec;
    std::filesystem::rename(temp_name, filename_, ec);
    if (ec) {
        std::error_code ignored;
        std::filesystem::remove(temp_name, ignored);
        std::cerr << "错误：无法用 " << temp_name << " 替换 "
                  << filename_ << "：" << ec.message() << std::endl;
        return false;
    }

    return true;
}

// ================================================================
//  从文件加载任务
// ================================================================

bool FileStorage::load(std::vector<Task>& out_tasks) {
    // 打开文件用于读取
    std::ifstream ifs(filename_);

    if (!ifs.is_open()) {
        // 文件不存在或不可读。调用方应先通过 exists() 区分"首次运行"；
        // 这里一律按失败处理，避免把"读不到"误当成"空列表"而覆盖数据。
        std::cerr << "错误：无法打开文件 " << filename_ << " 进行读取" << std::endl;
        return false;
    }

    // 先解析到临时列表，全部成功后再替换 out_tasks，
    // 避免读取出错时把调用者的数据改了一半。
    std::vector<Task> parsed;
    std::unordered_set<int> seen_ids;

    std::string line;
    bool first_line = true;
    while (std::getline(ifs, line)) {
        // 有些 Windows 编辑器会在 UTF-8 文件开头写 BOM，先去掉
        if (first_line) {
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }
            first_line = false;
        }

        // 兼容 CRLF 文件：文本模式下 Windows 通常会转换，
        // 但跨平台读取或手工编辑的文件可能仍带 '\r'
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // 跳过空行
        if (line.empty()) continue;

        Task task;
        if (!Task::deserialize(line, task)) {
            // 有坏行就整体失败，不加载"一半"，避免下次保存时丢掉坏行里的数据
            std::cerr << "错误：文件中有格式错误的行，已取消加载："
                      << line << std::endl;
            return false;
        }
        // ID 是任务的唯一标识；重复 ID 会让 delete/complete 只能命中其中一个，
        // 这样的文件本身就是损坏的，不能假装加载成功。
        if (!seen_ids.insert(task.getId()).second) {
            std::cerr << "错误：文件中存在重复的任务 ID #" << task.getId()
                      << "，已取消加载" << std::endl;
            return false;
        }
        parsed.push_back(task);
    }

    // bad() 表示发生了 I/O 错误（不是正常的文件结束）
    if (ifs.bad()) {
        std::cerr << "错误：读取文件 " << filename_ << " 失败" << std::endl;
        return false;
    }

    out_tasks = std::move(parsed);

    // ifs 在这里析构，自动关闭文件
    return true;
}

bool FileStorage::exists() const {
    // 能正常打开读取才认为数据源可用
    std::ifstream ifs(filename_);
    return ifs.good();
}

} // namespace todo
