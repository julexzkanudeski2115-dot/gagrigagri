#pragma once
#include <cstdint>
#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace adrenaline {

// "48 8B 0D ? ? ? ?" -> { 0x48, 0x8B, 0x0D, -1, -1, -1, -1 }
inline std::vector<int> parsePattern(const std::string& pattern) {
    std::vector<int> bytes;
    std::istringstream stream(pattern);
    std::string token;
    while (stream >> token) {
        if (token == "?" || token == "??") {
            bytes.push_back(-1);
        } else {
            bytes.push_back(static_cast<int>(std::stoul(token, nullptr, 16)));
        }
    }
    return bytes;
}

// Returns the index of the first match, or std::nullopt.
inline std::optional<size_t> findPattern(const uint8_t* data, size_t size, const std::vector<int>& pattern) {
    if (pattern.empty() || size < pattern.size()) {
        return std::nullopt;
    }
    const size_t last = size - pattern.size();
    for (size_t i = 0; i <= last; ++i) {
        bool matched = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j] != -1 && data[i + j] != static_cast<uint8_t>(pattern[j])) {
                matched = false;
                break;
            }
        }
        if (matched) {
            return i;
        }
    }
    return std::nullopt;
}

} // namespace adrenaline
