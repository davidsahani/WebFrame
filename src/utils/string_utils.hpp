#pragma once
#include <string>
#include <vector>

namespace StringUtils {

std::vector<std::string> Split(const std::string &str, char delimiter);
std::string Join(const std::vector<std::string> &tokens, char delimiter);
int TryParseInt(const std::string &str, int defaultValue);

} // namespace StringUtils