#include "string_utils.hpp"
#include <sstream>
#include <charconv>

std::vector<std::string> StringUtils::Split(const std::string &str, char delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        if (!token.empty()) result.push_back(token);
    }

    return result;
}

std::string StringUtils::Join(const std::vector<std::string> &tokens, char delimiter) {
    size_t size = tokens.size();
    if (size == 0) return "";

    std::ostringstream oss;
    oss << tokens[0];

    for (size_t i = 1; i < size; ++i) {
        oss << delimiter << tokens[i];
    }
    return oss.str();
}

int StringUtils::TryParseInt(const std::string &str, int defaultValue) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    return result.ec == std::errc() ? value : defaultValue;
}
