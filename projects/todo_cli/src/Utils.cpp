// ================================================================
//  Utils.cpp —— 工具函数实现
// ================================================================

#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <vector>

namespace todo {
namespace utils {

// ================================================================
//  转小写
// ================================================================

std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

// ================================================================
//  转大写
// ================================================================

std::string toUpper(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

// ================================================================
//  去除首尾空白
// ================================================================

std::string trim(const std::string& s) {
    size_t start = 0;
    size_t end = s.size();

    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(start, end - start);
}

// ================================================================
//  分割字符串
// ================================================================

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);

    while (std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }

    return tokens;
}

// ================================================================
//  拼接字符串
// ================================================================

std::string join(const std::vector<std::string>& parts, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) oss << delim;
        oss << parts[i];
    }
    return oss.str();
}

// ================================================================
//  判断前缀
// ================================================================

bool startsWith(const std::string& s, const std::string& prefix) {
    if (prefix.size() > s.size()) return false;
    return s.substr(0, prefix.size()) == prefix;
}

// ================================================================
//  判断后缀
// ================================================================

bool endsWith(const std::string& s, const std::string& suffix) {
    if (suffix.size() > s.size()) return false;
    return s.substr(s.size() - suffix.size()) == suffix;
}

// ================================================================
//  当前时间字符串
// ================================================================

std::string currentTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);

    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);

    return std::string(buf);
}

} // namespace utils
} // namespace todo
