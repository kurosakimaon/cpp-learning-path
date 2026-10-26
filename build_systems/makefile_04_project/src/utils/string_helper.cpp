#include "string_helper.h"

#include <cctype>

namespace demo {

std::string toUpper(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string join(const std::vector<std::string>& parts, const std::string& separator) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            result += separator;
        }
        result += parts[i];
    }
    return result;
}

} // namespace demo
