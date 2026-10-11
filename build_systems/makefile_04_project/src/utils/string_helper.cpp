/*
 * 示例4：字符串辅助工具实现
 */
#include "string_helper.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace utils {

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) return "";

    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); i++) {
        result += separator + parts[i];
    }
    return result;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }
    return str.substr(start, end - start);
}

std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

} // namespace utils
