#ifndef DEMO_STRING_HELPER_H
#define DEMO_STRING_HELPER_H

#include <string>
#include <vector>

namespace demo {

std::string toUpper(std::string text);
std::string join(const std::vector<std::string>& parts, const std::string& separator);

} // namespace demo

#endif // DEMO_STRING_HELPER_H
