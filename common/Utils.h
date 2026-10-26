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
    // __FUNCSIG__ 只有 MSVC 提供；GCC/Clang 使用 __PRETTY_FUNCTION__。
    // 两者签名格式不同，因此提取方式也不同。
#if defined(_MSC_VER)
    std::string name = __FUNCSIG__;
    // 形如：std::string __cdecl utils::type_name<int>(void)
    size_t begin = name.find("type_name<");
    if (begin == std::string::npos) return name;
    begin += 10;
    size_t end = name.rfind('>');
#elif defined(__GNUC__) || defined(__clang__)
    std::string name = __PRETTY_FUNCTION__;
    // GCC:   std::string utils::type_name() [with T = int; ...]
    // Clang: std::string utils::type_name() [T = int]
    size_t begin = name.find("T = ");
    if (begin == std::string::npos) return name;
    begin += 4;
    size_t end = name.find(';', begin);
    if (end == std::string::npos) end = name.find(']', begin);
#else
    return "unknown";
#endif

    if (end != std::string::npos && end > begin) {
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
