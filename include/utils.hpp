#pragma once

#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

inline int ToInt(std::string_view value) {
    // Trim leading and trailing whitespace
    auto start = value.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) {
        throw std::invalid_argument("Cannot convert '" + std::string(value) + "' to integral (empty or whitespace-only)");
    }
    auto end = value.find_last_not_of(" \t\n\r");
    value = value.substr(start, end - start + 1);

    int result{};
    auto [parse_end_ptr, error_code] = std::from_chars(value.begin(), value.end(), result);
    if (error_code != std::errc{} || parse_end_ptr != value.data() + value.size()) {
        throw std::invalid_argument("Cannot convert '" + std::string(value) + "' to integral");
    }
    return result;
}
