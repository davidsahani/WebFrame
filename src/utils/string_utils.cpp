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

std::string StringUtils::Trim(const std::string &str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;
    return str.substr(start, end - start);
}

int StringUtils::TryParseInt(const std::string &str, int defaultValue) {
    int value;
    std::string trimmed = Trim(str);
    auto result = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), value);
    return result.ec == std::errc() ? value : defaultValue;
}
