/**
 * 命令模式 (Command Pattern)
 *
 * 【模式意图】
 * 将一个请求封装为一个对象，从而使你可用不同的请求对客户进行参数化；
 * 对请求排队或记录请求日志，以及支持可撤销的操作。
 *
 * 【模式结构】
 * - Command: 抽象命令，声明执行操作的接口
 * - ConcreteCommand: 具体命令，将一个接收者对象绑定于一个动作
 * - Invoker: 调用者，要求该命令执行这个请求
 * - Receiver: 接收者，知道如何实施与执行一个请求相关的操作
 * - Client: 客户端，创建一个具体命令对象并设定它的接收者
 *
 * 【适用场景】
 * 1. 抽象出待执行的动作以参数化某对象
 * 2. 在不同的时刻指定、排列和执行请求
 * 3. 支持取消操作
 * 4. 支持修改日志，当系统崩溃时，这些修改可以被重做一遍
 * 5. 用构建在原语操作上的高层操作构造一个系统
 *
 * 【优点】
 * 1. 将调用操作的对象和知道如何执行该操作的对象解耦
 * 2. 命令是一等公民，可以像任何其他对象一样被操作和扩展
 * 3. 可以将多个命令装配成一个复合命令（宏命令）
 * 4. 增加新的命令很容易，无需修改已有类
 * 5. 可以实现撤销/重做、命令队列、命令日志等功能
 *
 * 【缺点】
 * 1. 可能导致某些系统有过多的具体命令类
 * 2. 增加了系统的复杂度
 *
 * 【现代C++实现方式】
 * 1. 经典命令模式：Command接口+具体命令类
 * 2. 撤销/重做：命令保存状态，支持undo/redo
 * 3. 命令队列：把提交与执行解耦，可配合工作线程实现异步调度
 * 4. 宏命令：组合多个命令
 * 5. std::function命令：函数式风格的轻量命令
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <functional>
#include <stack>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <cassert>

// ============================================================
// 接收者：文本编辑器
// ============================================================
class TextEditor {
public:
    void insertText(int position, const std::string& text) {
        buffer_.insert(position, text);
        std::cout << "  [TextEditor] 在位置 " << position
                  << " 插入: \"" << text << "\"" << std::endl;
    }

    void deleteText(int position, int length) {
        deletedText_ = buffer_.substr(position, length);
        deletedPosition_ = position;
        buffer_.erase(position, length);
        std::cout << "  [TextEditor] 从位置 " << position
                  << " 删除 " << length << " 个字符: \""
                  << deletedText_ << "\"" << std::endl;
    }

    void replaceText(int position, int length, const std::string& newText) {
        originalText_ = buffer_.substr(position, length);
        originalPosition_ = position;
        originalLength_ = length;
        buffer_.replace(position, length, newText);
        std::cout << "  [TextEditor] 替换位置 " << position
                  << " 的 " << length << " 个字符为: \"" << newText << "\"" << std::endl;
    }

    const std::string& getBuffer() const { return buffer_; }

    // 用于撤销的辅助信息
    const std::string& getLastDeletedText() const { return deletedText_; }
    int getLastDeletedPosition() const { return deletedPosition_; }
    const std::string& getLastOriginalText() const { return originalText_; }
    int getLastOriginalPosition() const { return originalPosition_; }
    int getLastOriginalLength() const { return originalLength_; }

private:
    std::string buffer_ = "";
    std::string deletedText_;
    int deletedPosition_ = 0;
    std::string originalText_;
    int originalPosition_ = 0;
    int originalLength_ = 0;
};

// ============================================================
// 经典命令模式
// ============================================================

// 抽象命令
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
};

// 具体命令：插入文本
class InsertCommand : public ICommand {
public:
    InsertCommand(TextEditor& editor, int position, std::string text)
        : editor_(editor), position_(position), text_(std::move(text)) {
    }

    void execute() override {
        editor_.insertText(position_, text_);
    }

    void undo() override {
        // 撤销插入 = 删除插入的文本
        editor_.deleteText(position_, static_cast<int>(text_.size()));
        std::cout << "  [Undo] 撤销插入操作" << std::endl;
    }

    std::string getName() const override {
        return "插入(" + text_ + ")";
    }

private:
    TextEditor& editor_;
    int position_;
    std::string text_;
};

// 具体命令：删除文本
class DeleteCommand : public ICommand {
public:
    DeleteCommand(TextEditor& editor, int position, int length)
        : editor_(editor), position_(position), length_(length) {
    }

    void execute() override {
        editor_.deleteText(position_, length_);
        deletedText_ = editor_.getLastDeletedText();
    }

    void undo() override {
        // 撤销删除 = 重新插入被删除的文本
        editor_.insertText(position_, deletedText_);
        std::cout << "  [Undo] 撤销删除操作" << std::endl;
    }

    std::string getName() const override {
        return "删除(" + std::to_string(length_) + "字符)";
    }

private:
    TextEditor& editor_;
    int position_;
    int length_;
    std::string deletedText_;  // 保存被删除的文本用于撤销
};

// 具体命令：替换文本
class ReplaceCommand : public ICommand {
public:
    ReplaceCommand(TextEditor& editor, int position, int length, std::string newText)
        : editor_(editor), position_(position), length_(length), newText_(std::move(newText)) {
    }

    void execute() override {
        editor_.replaceText(position_, length_, newText_);
        originalText_ = editor_.getLastOriginalText();
    }

    void undo() override {
        // 撤销替换 = 替换回原文
        editor_.replaceText(position_, static_cast<int>(newText_.size()), originalText_);
        std::cout << "  [Undo] 撤销替换操作" << std::endl;
    }

    std::string getName() const override {
        return "替换";
    }

private:
    TextEditor& editor_;
    int position_;
    int length_;
    std::string newText_;
    std::string originalText_;
};

// ============================================================
// 2. 撤销/重做管理器
// ============================================================

/**
 * 撤销/重做是命令模式最经典的应用场景。
 * 使用两个栈：
 * - undoStack: 已执行的命令，用于撤销
 * - redoStack: 已撤销的命令，用于重做
 */
class UndoRedoManager {
public:
    // 执行命令
    void executeCommand(std::unique_ptr<ICommand> command) {
        command->execute();
        undoStack_.push_back(std::move(command));
        // 执行新命令后清空重做栈
        redoStack_.clear();
    }

    // 撤销
    bool undo() {
        if (undoStack_.empty()) {
            std::cout << "  无法撤销：撤销栈为空" << std::endl;
            return false;
        }

        auto command = std::move(undoStack_.back());
        undoStack_.pop_back();
        std::cout << "  [撤销] " << command->getName() << std::endl;
        command->undo();
        redoStack_.push_back(std::move(command));
        return true;
    }

    // 重做
    bool redo() {
        if (redoStack_.empty()) {
            std::cout << "  无法重做：重做栈为空" << std::endl;
            return false;
        }

        auto command = std::move(redoStack_.back());
        redoStack_.pop_back();
        std::cout << "  [重做] " << command->getName() << std::endl;
        command->execute();
        undoStack_.push_back(std::move(command));
        return true;
    }

    size_t getUndoCount() const { return undoStack_.size(); }
    size_t getRedoCount() const { return redoStack_.size(); }

private:
    std::vector<std::unique_ptr<ICommand>> undoStack_;
    std::vector<std::unique_ptr<ICommand>> redoStack_;
};

// ============================================================
// 3. 命令队列（延后 / 批处理执行）
// ============================================================

/**
 * 命令队列：把“提交命令”和“执行命令”解耦。
 * 调用者只负责入队，再由 processOne/processAll 在调用线程中执行。
 * 这是任务调度、线程池、消息队列的基础：
 * 如果另起一个工作线程不断调用 processOne，就得到了异步执行。
 */

class CommandQueue {
public:
    // 添加命令到队列
    void enqueue(std::function<void()> command) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(command));
    }

    // 执行队列中的所有命令
    void processAll() {
        size_t count = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            count = queue_.size();
        }
        std::cout << "  [命令队列] 处理 " << count << " 个命令" << std::endl;

        while (true) {
            std::function<void()> command;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (queue_.empty()) break;
                command = std::move(queue_.front());
                queue_.pop_front();
            }
            // 在锁外执行：命令内部可以继续 enqueue，也不会长时间占着锁
            command();
        }
    }

    // 处理一个命令
    bool processOne() {
        std::function<void()> command;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) return false;
            command = std::move(queue_.front());
            queue_.pop_front();
        }
        command();
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::deque<std::function<void()>> queue_;
};

// ============================================================
// 4. 宏命令（组合命令）
// ============================================================

/**
 * 宏命令：将多个命令组合成一个命令。
 * 这是组合模式在命令模式中的应用。
 */
class MacroCommand : public ICommand {
public:
    void addCommand(std::unique_ptr<ICommand> command) {
        commands_.push_back(std::move(command));
    }

    void execute() override {
        std::cout << "  [宏命令] 开始执行宏 (" << commands_.size() << " 个子命令)" << std::endl;
        for (auto& cmd : commands_) {
            cmd->execute();
        }
        std::cout << "  [宏命令] 宏执行完成" << std::endl;
    }

    void undo() override {
        std::cout << "  [宏命令] 开始撤销宏" << std::endl;
        // 注意：撤销顺序与执行顺序相反
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
        std::cout << "  [宏命令] 宏撤销完成" << std::endl;
    }

    std::string getName() const override {
        return "宏命令(" + std::to_string(commands_.size()) + "个子命令)";
    }

private:
    std::vector<std::unique_ptr<ICommand>> commands_;
};

// ============================================================
// 5. std::function 命令（函数式风格）
// ============================================================

/**
 * 使用std::function和lambda可以实现非常轻量级的命令模式。
 * 不需要定义Command接口和具体命令类。
 *
 * 适用于简单场景，特别是不需要撤销功能的情况。
 */

// 轻量级命令类型
using LightCommand = std::function<void()>;

// 调用者：按钮
class LightButton {
public:
    void setCommand(LightCommand command) {
        command_ = std::move(command);
    }

    void click() {
        if (command_) {
            command_();
        }
    }

private:
    LightCommand command_;
};

// 接收者：灯
class Light {
public:
    void turnOn() {
        isOn_ = true;
        std::cout << "  [Light] 灯已打开" << std::endl;
    }

    void turnOff() {
        isOn_ = false;
        std::cout << "  [Light] 灯已关闭" << std::endl;
    }

    bool isOn() const { return isOn_; }

private:
    bool isOn_ = false;
};

// ============================================================
// 6. 命令模式的实际应用：文件操作
// ============================================================

// 文件系统接收者
class FileSystem {
public:
    void createFile(const std::string& path, const std::string& content) {
        files_[path] = content;
        std::cout << "  [FileSystem] 创建文件: " << path << std::endl;
    }

    void deleteFile(const std::string& path) {
        auto it = files_.find(path);
        if (it != files_.end()) {
            lastDeletedContent_ = it->second;
            files_.erase(it);
        }
        std::cout << "  [FileSystem] 删除文件: " << path << std::endl;
    }

    void modifyFile(const std::string& path, const std::string& newContent) {
        auto it = files_.find(path);
        if (it != files_.end()) {
            lastOriginalContent_ = it->second;
            it->second = newContent;
        }
        std::cout << "  [FileSystem] 修改文件: " << path << std::endl;
    }

    std::string readFile(const std::string& path) const {
        auto it = files_.find(path);
        if (it != files_.end()) return it->second;
        return "";
    }

    bool fileExists(const std::string& path) const {
        return files_.find(path) != files_.end();
    }

    const std::string& getLastDeletedContent() const { return lastDeletedContent_; }
    const std::string& getLastOriginalContent() const { return lastOriginalContent_; }

private:
    std::map<std::string, std::string> files_;
    std::string lastDeletedContent_;
    std::string lastOriginalContent_;
};

// 创建文件命令
class CreateFileCommand : public ICommand {
public:
    CreateFileCommand(FileSystem& fs, std::string path, std::string content)
        : fs_(fs), path_(std::move(path)), content_(std::move(content)) {}

    void execute() override {
        fs_.createFile(path_, content_);
    }

    void undo() override {
        fs_.deleteFile(path_);
        std::cout << "  [Undo] 撤销创建文件" << std::endl;
    }

    std::string getName() const override { return "创建文件 " + path_; }

private:
    FileSystem& fs_;
    std::string path_;
    std::string content_;
};

// 删除文件命令
class DeleteFileCommand : public ICommand {
public:
    DeleteFileCommand(FileSystem& fs, std::string path)
        : fs_(fs), path_(std::move(path)) {}

    void execute() override {
        fs_.deleteFile(path_);
        deletedContent_ = fs_.getLastDeletedContent();
    }

    void undo() override {
        fs_.createFile(path_, deletedContent_);
        std::cout << "  [Undo] 撤销删除文件" << std::endl;
    }

    std::string getName() const override { return "删除文件 " + path_; }

private:
    FileSystem& fs_;
    std::string path_;
    std::string deletedContent_;
};

// ============================================================
// 测试代码
// ============================================================

void testClassicCommand() {
    std::cout << "\n========== 经典命令模式测试 ==========" << std::endl;

    TextEditor editor;
    UndoRedoManager manager;

    std::cout << "\n--- 执行一系列编辑操作 ---" << std::endl;
    manager.executeCommand(std::make_unique<InsertCommand>(editor, 0, "Hello"));
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    manager.executeCommand(std::make_unique<InsertCommand>(editor, 5, " World"));
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    manager.executeCommand(std::make_unique<DeleteCommand>(editor, 5, 6));
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    manager.executeCommand(std::make_unique<ReplaceCommand>(editor, 0, 5, "Hi"));
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    std::cout << "\n--- 撤销操作 ---" << std::endl;
    manager.undo();  // 撤销替换
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    manager.undo();  // 撤销删除
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    std::cout << "\n--- 重做操作 ---" << std::endl;
    manager.redo();  // 重做删除
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    manager.redo();  // 重做替换
    std::cout << "  当前文本: \"" << editor.getBuffer() << "\"" << std::endl;

    std::cout << "\n  撤销/重做次数: undo=" << manager.getUndoCount()
              << ", redo=" << manager.getRedoCount() << std::endl;
}

void testCommandQueue() {
    std::cout << "\n========== 命令队列测试 ==========" << std::endl;

    CommandQueue queue;

    // 添加命令到队列
    std::cout << "\n--- 添加命令到队列 ---" << std::endl;
    queue.enqueue([]() { std::cout << "    任务1: 加载配置" << std::endl; });
    queue.enqueue([]() { std::cout << "    任务2: 初始化数据库" << std::endl; });
    queue.enqueue([]() { std::cout << "    任务3: 启动服务" << std::endl; });
    queue.enqueue([]() { std::cout << "    任务4: 注册到服务发现" << std::endl; });

    std::cout << "  队列中命令数: " << queue.size() << std::endl;

    // 逐个执行
    std::cout << "\n--- 逐个执行命令 ---" << std::endl;
    while (queue.processOne()) {
        std::cout << "    (剩余 " << queue.size() << " 个命令)" << std::endl;
    }

    // 批量执行
    std::cout << "\n--- 批量执行 ---" << std::endl;
    queue.enqueue([]() { std::cout << "    批处理任务A" << std::endl; });
    queue.enqueue([]() { std::cout << "    批处理任务B" << std::endl; });
    queue.enqueue([]() { std::cout << "    批处理任务C" << std::endl; });
    queue.processAll();
}

void testMacroCommand() {
    std::cout << "\n========== 宏命令测试 ==========" << std::endl;

    TextEditor editor;

    // 创建一个宏命令：初始化文档模板
    // 注意：所有子命令在执行前就已创建，所以插入位置必须按最终顺序预先算好，
    // 不能在这里用 editor.getBuffer().size()——那时缓冲区还是空的。
    auto macro = std::make_unique<MacroCommand>();
    int position = 0;
    auto addInsert = [&](const std::string& text) {
        macro->addCommand(std::make_unique<InsertCommand>(editor, position, text));
        position += static_cast<int>(text.size());
    };
    addInsert("标题：项目报告\n");
    addInsert("作者：张三\n");
    addInsert("日期：2024-01-15\n");
    addInsert("====================\n");

    std::cout << "\n--- 执行宏命令 ---" << std::endl;
    macro->execute();
    std::cout << "  文档内容:\n" << editor.getBuffer() << std::endl;

    std::cout << "--- 撤销宏命令 ---" << std::endl;
    macro->undo();
    std::cout << "  文档内容: \"" << editor.getBuffer() << "\"" << std::endl;
}

void testLightCommand() {
    std::cout << "\n========== std::function轻量命令测试 ==========" << std::endl;

    Light livingRoomLight;
    LightButton onButton;
    LightButton offButton;
    LightButton toggleButton;

    // 绑定命令
    onButton.setCommand([&livingRoomLight]() {
        livingRoomLight.turnOn();
    });

    offButton.setCommand([&livingRoomLight]() {
        livingRoomLight.turnOff();
    });

    toggleButton.setCommand([&livingRoomLight]() {
        if (livingRoomLight.isOn()) {
            livingRoomLight.turnOff();
        } else {
            livingRoomLight.turnOn();
        }
    });

    std::cout << "\n--- 点击开灯按钮 ---" << std::endl;
    onButton.click();
    std::cout << "  灯状态: " << (livingRoomLight.isOn() ? "开" : "关") << std::endl;

    std::cout << "\n--- 点击切换按钮 ---" << std::endl;
    toggleButton.click();
    std::cout << "  灯状态: " << (livingRoomLight.isOn() ? "开" : "关") << std::endl;

    std::cout << "\n--- 再次点击切换按钮 ---" << std::endl;
    toggleButton.click();
    std::cout << "  灯状态: " << (livingRoomLight.isOn() ? "开" : "关") << std::endl;

    std::cout << "\n  std::function命令：轻量、灵活，适合简单场景" << std::endl;
}

void testFileOperations() {
    std::cout << "\n========== 文件操作命令测试 ==========" << std::endl;

    FileSystem fs;
    UndoRedoManager manager;

    std::cout << "\n--- 创建文件 ---" << std::endl;
    manager.executeCommand(std::make_unique<CreateFileCommand>(fs, "/docs/readme.txt", "Hello World"));
    std::cout << "  文件存在: " << (fs.fileExists("/docs/readme.txt") ? "是" : "否") << std::endl;
    std::cout << "  文件内容: " << fs.readFile("/docs/readme.txt") << std::endl;

    std::cout << "\n--- 撤销创建 ---" << std::endl;
    manager.undo();
    std::cout << "  文件存在: " << (fs.fileExists("/docs/readme.txt") ? "是" : "否") << std::endl;

    std::cout << "\n--- 重做创建 ---" << std::endl;
    manager.redo();
    std::cout << "  文件存在: " << (fs.fileExists("/docs/readme.txt") ? "是" : "否") << std::endl;

    std::cout << "\n--- 删除文件 ---" << std::endl;
    manager.executeCommand(std::make_unique<DeleteFileCommand>(fs, "/docs/readme.txt"));
    std::cout << "  文件存在: " << (fs.fileExists("/docs/readme.txt") ? "是" : "否") << std::endl;

    std::cout << "\n--- 撤销删除（文件恢复） ---" << std::endl;
    manager.undo();
    std::cout << "  文件存在: " << (fs.fileExists("/docs/readme.txt") ? "是" : "否") << std::endl;
    std::cout << "  文件内容: " << fs.readFile("/docs/readme.txt") << std::endl;
}

int main() {
    std::cout << "========== 命令模式学习 Demo ==========" << std::endl;

    testClassicCommand();
    testCommandQueue();
    testMacroCommand();
    testLightCommand();
    testFileOperations();

    std::cout << "\n========== 命令模式总结 ==========" << std::endl;
    std::cout << "核心思想：将请求封装为对象，实现调用者和执行者解耦" << std::endl;
    std::cout << "经典命令：Command接口 + 具体命令类，支持撤销/重做" << std::endl;
    std::cout << "撤销/重做：双栈结构，命令对象保存状态" << std::endl;
    std::cout << "命令队列：延后执行、任务调度、批处理" << std::endl;
    std::cout << "宏命令：组合模式 + 命令模式，多个命令组合执行" << std::endl;
    std::cout << "std::function：函数式风格，轻量级命令" << std::endl;
    std::cout << "设计原则：单一职责、开闭原则" << std::endl;

    return 0;
}
