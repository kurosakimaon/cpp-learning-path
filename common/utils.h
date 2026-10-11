// ============================================================================
// 公共工具头文件 - C++学习Demo通用工具
// ============================================================================
#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>

// ============================================================================
// 输出分隔与标题工具
// ============================================================================
namespace utils {

inline std::mutex& output_mutex() {
    static std::mutex mtx;
    return mtx;
}

inline void print_title(const std::string& title) {
    std::lock_guard<std::mutex> lock(output_mutex());
    const size_t width = 70;
    const size_t padding = (width - title.size() - 2) / 2;
    std::cout << "\n" << std::string(width, '=') << "\n";
    std::cout << std::string(padding, ' ') << " " << title << " "
              << std::string(width - padding - title.size() - 2, ' ') << "\n";
    std::cout << std::string(width, '=') << "\n";
}

inline void print_subtitle(const std::string& subtitle) {
    std::lock_guard<std::mutex> lock(output_mutex());
    std::cout << "\n--- " << subtitle << " ---\n";
}

inline void print_note(const std::string& note) {
    std::lock_guard<std::mutex> lock(output_mutex());
    std::cout << "[NOTE] " << note << "\n";
}

// ============================================================================
// 性能计时器
// ============================================================================
class Timer {
public:
    Timer(const std::string& name = "") : name_(name) {
        start_ = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        if (!stopped_) {
            stop();
        }
    }

    void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        stopped_ = true;
        std::lock_guard<std::mutex> lock(output_mutex());
        if (!name_.empty()) {
            std::cout << "[Timer] " << name_ << ": "
                      << std::fixed << std::setprecision(3)
                      << duration.count() / 1000.0 << " ms\n";
        }
    }

    template <typename Duration = std::chrono::microseconds>
    typename Duration::rep elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<Duration>(end - start_).count();
    }

private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
    bool stopped_ = false;
};

// ============================================================================
// 线程安全输出
// ============================================================================
class ThreadSafePrinter {
public:
    ThreadSafePrinter() : ss_() {}

    ~ThreadSafePrinter() {
        std::lock_guard<std::mutex> lock(output_mutex());
        std::cout << ss_.str();
    }

    template <typename T>
    ThreadSafePrinter& operator<<(const T& value) {
        ss_ << value;
        return *this;
    }

    ThreadSafePrinter& operator<<(std::ostream& (*manip)(std::ostream&)) {
        ss_ << manip;
        return *this;
    }

private:
    std::ostringstream ss_;
};

#define TS_PRINT utils::ThreadSafePrinter()

// ============================================================================
// 类型名打印（用于模板元编程调试）
// ============================================================================
template <typename T>
std::string type_name() {
    std::string name = __FUNCSIG__;
    // 从函数签名中提取类型名
    // 不同编译器格式略有不同，这里做通用处理
    size_t begin = name.find("type_name<") + 10;
    size_t end = name.rfind(">");
    if (begin != std::string::npos && end != std::string::npos && end > begin) {
        return name.substr(begin, end - begin);
    }
    return name;
}

// ============================================================================
// 辅助：打印容器
// ============================================================================
template <typename Container>
void print_container(const Container& c, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": ";
    }
    std::cout << "[";
    bool first = true;
    for (const auto& elem : c) {
        if (!first) std::cout << ", ";
        std::cout << elem;
        first = false;
    }
    std::cout << "]\n";
}

} // namespace utils
