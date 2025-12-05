#include "aoc.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>   // std::exit
#include <iostream>
#include <stdexcept>
#include <string>

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <algorithm>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day05.txt";
}

struct Range {
    long long start;
    long long end;
};

long long parse_ll(std::string_view sv) {
    long long value{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        throw std::runtime_error("invalid integer: " + std::string(sv));
    }
    return value;
}

Range parse_range_line(std::string_view line) {
    auto dash_pos = line.find('-');
    if (dash_pos == std::string_view::npos) {
        throw std::runtime_error("invalid range line (no '-'): " + std::string(line));
    }

    std::string_view left  = line.substr(0, dash_pos);
    std::string_view right = line.substr(dash_pos + 1);

    Range r{
        .start = parse_ll(left),
        .end   = parse_ll(right),
    };

    if (r.start > r.end) {
        throw std::runtime_error("range start > end: " + std::string(line));
    }

    return r;
}

bool in_range(long long value, Range const& range) {
    return value >= range.start && value <= range.end;
}

long long solve_part1(const std::vector<std::string> &lines) {
    long long fresh{0};
    std::vector<Range> ranges;

    bool reading_ranges = true;

    for (auto const& line : lines) {
        if (line.empty()) {
            // Empty line switches to IDs section
            reading_ranges = false;
            // Print ranges for debugging
            // for (const auto& range : ranges) {
            //     std::cout << "Range: " << range.start << "-" << range.end
            //     << "\n";
            // }
            continue;
        }

        // Store the ranges
        if (reading_ranges) {
            ranges.push_back(parse_range_line(line));
        }
        // Checking IDs
        else {
            long long ID = parse_ll(line);
            // std::cout << "ID: " << ID << std::endl;
            for (const auto& range : ranges) {
                if (in_range(ID, range)) {
                    ++fresh;
                    break;
                }
            }
        }
    }
    return fresh;
}

long long solve_part2(const std::vector<std::string> &lines) {
    long long fresh{0};
    std::vector<Range> ranges;

    for (auto const& line : lines) {
        if (line.empty()) {
            // Print ranges for debugging
            // for (const auto& range : ranges) {
            //     std::cout << "Range: " << range.start << "-" << range.end
            //     << "\n";
            // }
            break;
        }

        // Store the ranges
        ranges.push_back(parse_range_line(line));
    }

    // Sort the ranges by start value
    std::sort(ranges.begin(), ranges.end(), [](Range const& a, Range const& b) {
        return a.start < b.start;
    });

    std::vector<Range> processed_ranges;

    // Merge overlapping ranges
    for (const auto& range : ranges) {
        if (processed_ranges.empty() ||
            // Start of new range
            range.start > processed_ranges.back().end) {
            processed_ranges.push_back(range);
        }
        // Check what is the biggest end between current and new
        else {
            processed_ranges.back().end =
                std::max(processed_ranges.back().end, range.end);
        }
        // if (processed_ranges.empty() ||
        //     range.start > processed_ranges.back().end) {
        //     processed_ranges.push_back(range);
        // } else {
        //     processed_ranges.back().end =
        //         std::max(processed_ranges.back().end, range.end);
        // }
    }

    // Clear raw ranges because no longer neededd
    ranges.clear();

    // Calculate total fresh IDs
    for (const auto& range : processed_ranges) {
        fresh += (range.end - range.start + 1);
    }

    return fresh;
}

int main(int argc, char **argv) {
    try {
        const auto input_path = day_input_path(argc, argv);
        auto lines = read_lines(input_path);

        {
            auto [ans1, t1] = time_it([&] { return solve_part1(lines); });
            print_answer(1, ans1, t1);
        }

        {
            auto [ans2, t2] = time_it([&] { return solve_part2(lines); });
            print_answer(2, ans2, t2);
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}