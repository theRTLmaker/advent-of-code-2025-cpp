#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>
#include <string>

#include <charconv>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day02.txt";
}

struct Range {
    int unsigned start;
    int unsigned end;
};

int unsigned parse_ll(std::string_view sv) {
    int unsigned value{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        throw std::runtime_error("invalid integer: " + std::string(sv));
    }
    return value;
}

std::vector<Range> parse_ranges(std::string_view input) {
    std::vector<Range> result;

    auto comma_split = input | std::views::split(',');
    for (auto&& part : comma_split) {
        std::string_view token{&*part.begin(), static_cast<std::size_t>(std::ranges::distance(part))};

        auto dash_split = token | std::views::split('-');

        std::vector<std::string_view> pieces;
        for (auto&& piece : dash_split) {
            pieces.emplace_back(&*piece.begin(),
                                static_cast<std::size_t>(std::ranges::distance(piece)));
        }
        if (pieces.size() != 2) {
            throw std::runtime_error("invalid range: " + std::string(token));
        }

        result.push_back(Range{
            .start = parse_ll(pieces[0]),
            .end   = parse_ll(pieces[1]),
        });
    }
    return result;
}

bool in_range(int unsigned value, Range const& range) {
    return value >= range.start && value <= range.end;
}

static bool is_invalid(int unsigned value) {
    std::string sv = std::to_string(value);
    // Odd don't have a middle part
    if (sv.size() % 2 == 1)  {
        return false;
    }

    std::size_t mid = sv.size() / 2;
    return sv.substr(0, mid) == sv.substr(mid, mid);
}

long long solve_part1(const std::vector<std::string> &lines) {
    long long acc = 0;
    for (const auto &line : lines) {
        auto ranges = parse_ranges(line);

        for (auto const& r : ranges) {
            // std::cout << r.start << " .. " << r.end << "\n";

            for (int unsigned test_value = r.start; test_value <= r.end; ++test_value) {
                // std::cout << "  Testing value: " << test_value << "\n";
                if (is_invalid(test_value)) {
                    // std::cout << "    Invalid!\n";
                    acc += test_value;
                }
            }
        }
    }
    return acc;
}

static int unsigned countDistinctDigits(const std::string &s) {
    bool seen[10] = {false};
    int unsigned count = 0;

    for (char c : s) {
        if (c < '0' || c > '9') {
            continue; // or handle error if you want strict numeric
        }
        int idx = c - '0';
        if (!seen[idx]) {
            seen[idx] = true;
            ++count;
        }
    }

    return count;
}


static bool is_invalid_part2(unsigned int value) {
    std::string sv = std::to_string(value);
    std::string_view sv_view{sv};

    unsigned int distinct_digits = countDistinctDigits(sv);

    // Single digit numbers are always invalid
    if (sv_view.size() > 1 && distinct_digits == 1) {
        return true;
    }
    // If more than half the digits are distinct, can't be repeating
    else if (distinct_digits > sv_view.size() / 2) {
        return false;
    }

    for (size_t slice = 2; slice <= sv_view.size() / 2; ++slice) {
        if (sv_view.size() % slice != 0) {
            continue;
        }

        bool all_match{true};
        std::string_view first_part{sv_view.substr(0, slice)};

        for (size_t pos = slice; pos < sv_view.size(); pos += slice) {
            if (sv_view.substr(pos, slice) != first_part) {
                all_match = false;
                break;
            }
        }

        if (all_match) {
            return true;
        }
    }

    return false;
}

long long solve_part2(const std::vector<std::string> &lines) {
    long long acc = 0;
    for (const auto &line : lines) {
        auto ranges = parse_ranges(line);

        for (auto const& r : ranges) {
            // std::cout << r.start << " .. " << r.end << "\n";
            for (int unsigned test_value = r.start; test_value <= r.end; ++test_value) {
                // std::cout << "  Testing value: " << test_value << "\n";
                if (is_invalid_part2(test_value)) {
                    // std::cout << "    Invalid!\n";
                    acc += test_value;
                }
            }
        }
    }
    return acc;
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