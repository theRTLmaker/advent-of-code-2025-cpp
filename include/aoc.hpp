#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace aoc {

inline std::vector<std::string> read_lines(const std::filesystem::path &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Failed to open input file: " + path.string());
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
    }
    return lines;
}

inline std::string trim(std::string_view sv) {
    std::size_t start = 0;
    while (start < sv.size() && std::isspace(static_cast<unsigned char>(sv[start]))) {
        ++start;
    }
    std::size_t end = sv.size();
    while (end > start && std::isspace(static_cast<unsigned char>(sv[end - 1]))) {
        --end;
    }
    return std::string(sv.substr(start, end - start));
}

// Tiny timing helper
template <typename F>
auto time_it(F &&f) {
    auto start = std::chrono::steady_clock::now();
    auto result = f();
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return std::pair{result, ms};
}

inline void print_answer(int part, const auto &answer, long long micros) {
    std::cout << "Part " << part << ": " << answer
              << " (" << micros << " us)\n";
}

}  // namespace aoc