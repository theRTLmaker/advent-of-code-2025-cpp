#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>

#include <bit>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <ostream>
#include <limits>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day10.txt";
}

using Pattern = std::vector<std::uint16_t>;
using Groups  = std::vector<std::vector<std::uint16_t>>;
using Values  = std::vector<std::uint16_t>;

struct Row {
    Pattern pattern;      // from [...]
    Groups  groups;       // from each (...)
    Values  values;       // from {...}
};

std::ostream& operator<<(std::ostream& os, const Row& row) {
    os << "pattern: [";
    for (std::size_t i = 0; i < row.pattern.size(); ++i) {
        os << static_cast<int>(row.pattern[i]);
        if (i + 1 < row.pattern.size()) {
            os << ",";
        }
    }
    os << "]  groups: [";

    for (std::size_t g = 0; g < row.groups.size(); ++g) {
        os << "(";
        const auto& grp = row.groups[g];
        for (std::size_t i = 0; i < grp.size(); ++i) {
            os << static_cast<int>(grp[i]);
            if (i + 1 < grp.size()) {
                os << ",";
            }
        }
        os << ")";
        if (g + 1 < row.groups.size()) {
            os << " ";
        }
    }

    os << "]  values: {";
    for (std::size_t i = 0; i < row.values.size(); ++i) {
        os << static_cast<int>(row.values[i]);
        if (i + 1 < row.values.size()) {
            os << ",";
        }
    }
    os << "}";

    return os;
}

// single 16-bit parser used everywhere
std::uint16_t parse_u16(std::string_view sv) {
    sv = trim(sv);
    if (sv.empty()) {
        throw std::runtime_error("empty integer field");
    }

    unsigned int tmp{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), tmp);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        throw std::runtime_error("invalid integer: " + std::string(sv));
    }
    if (tmp > 65535u) {
        throw std::runtime_error("integer out of uint16 range: " + std::to_string(tmp));
    }
    return static_cast<std::uint16_t>(tmp);
}

Pattern parse_pattern(std::string_view line) {
    auto open  = line.find('[');
    auto close = line.find(']', open == std::string_view::npos ? 0 : open + 1);
    if (open == std::string_view::npos || close == std::string_view::npos || close <= open + 1) {
        throw std::runtime_error("invalid pattern segment in line: " + std::string(line));
    }

    std::string_view inside = line.substr(open + 1, close - open - 1);

    Pattern pattern;
    pattern.reserve(inside.size());
    for (char c : inside) {
        switch (c) {
        case '.':
            pattern.push_back(static_cast<std::uint16_t>(0));
            break;
        case '#':
            pattern.push_back(static_cast<std::uint16_t>(1));
            break;
        default:
            throw std::runtime_error("invalid char in pattern: " + std::string(1, c));
        }
    }
    return pattern;
}

Groups parse_groups(std::string_view line) {
    Groups groups;

    // stop at first '{', groups are before that
    auto brace_pos = line.find('{');
    std::string_view section = (brace_pos == std::string_view::npos)
                               ? line
                               : line.substr(0, brace_pos);

    std::size_t pos = 0;
    while (pos < section.size()) {
        auto open = section.find('(', pos);
        if (open == std::string_view::npos) {
            break;
        }
        auto close = section.find(')', open + 1);
        if (close == std::string_view::npos) {
            throw std::runtime_error("unmatched '(' in line: " + std::string(line));
        }

        std::string_view inside = section.substr(open + 1, close - open - 1);
        inside = trim(inside);

        std::vector<std::uint16_t> group;
        if (!inside.empty()) {
            std::size_t ipos = 0;
            while (ipos < inside.size()) {
                auto comma = inside.find(',', ipos);
                std::string_view tok = (comma == std::string_view::npos)
                                       ? inside.substr(ipos)
                                       : inside.substr(ipos, comma - ipos);
                tok = trim(tok);
                if (!tok.empty()) {
                    group.push_back(parse_u16(tok));
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                ipos = comma + 1;
            }
        }

        groups.push_back(std::move(group));
        pos = close + 1;
    }

    return groups;
}

Values parse_braced_values(std::string_view line) {
    auto open  = line.find('{');
    auto close = line.find('}', open == std::string_view::npos ? 0 : open + 1);
    if (open == std::string_view::npos || close == std::string_view::npos || close <= open + 1) {
        throw std::runtime_error("invalid brace segment in line: " + std::string(line));
    }

    std::string_view inside = line.substr(open + 1, close - open - 1);
    Values values;

    std::size_t pos = 0;
    while (pos < inside.size()) {
        std::size_t comma = inside.find(',', pos);
        std::string_view token = (comma == std::string_view::npos)
                                 ? inside.substr(pos)
                                 : inside.substr(pos, comma - pos);
        token = trim(token);
        if (!token.empty()) {
            values.push_back(parse_u16(token));
        }
        if (comma == std::string_view::npos) {
            break;
        }
        pos = comma + 1;
    }

    return values;
}

Row parse_line(const std::string& line_str) {
    std::string_view line{line_str};

    Row row;
    row.pattern = parse_pattern(line);
    row.groups  = parse_groups(line);
    row.values  = parse_braced_values(line);

    return row;
}

// Return how many groups are used by this mask if it matches pattern via XOR, -1 otherwise
int used_groups_if_matches(const Groups& groups,
                           const Pattern& pattern,
                           std::uint64_t mask)
{
    const std::size_t G = groups.size();
    Pattern accum(pattern.size(), static_cast<std::uint16_t>(0));

    int used = 0;

    for (std::size_t g = 0; g < G; ++g) {
        if ((mask & (1ull << g)) == 0) {
            continue;
        }
        ++used;

        for (std::uint16_t idx : groups[g]) {
            if (idx >= accum.size()) {
                throw std::runtime_error("group index out of bounds");
            }
            // toggle light (XOR / parity), still just 0/1 stored in uint16
            accum[static_cast<std::size_t>(idx)] ^= static_cast<std::uint16_t>(1);
        }
    }

    if (accum == pattern) {
        return used;
    }
    return -1;
}

int min_groups_to_match(const Groups& groups,
                        const Pattern& pattern)
{
    const std::size_t G = groups.size();
    if (G == 0) {
        // no groups; only valid if pattern is all zeros
        bool all_zero = true;
        for (auto v : pattern) {
            if (v != 0) {
                all_zero = false;
                break;
            }
        }
        return all_zero ? 0 : -1;
    }

    if (G >= 63) {
        throw std::runtime_error("too many groups for brute-force bitmask");
    }

    int best = std::numeric_limits<int>::max();
    const std::uint64_t total_masks = 1ull << G;

    for (std::uint64_t mask = 1; mask < total_masks; ++mask) { // skip mask 0
        int bits = std::popcount(mask);
        if (bits >= best) {
            continue; // prune: we already have a better or equal solution
        }

        int used = used_groups_if_matches(groups, pattern, mask);
        if (used != -1 && used < best) {
            best = used;
        }
    }

    if (best == std::numeric_limits<int>::max()) {
        return -1; // no combination matches
    }
    return best;
}

long long solve_part1(const std::vector<std::string> &lines) {
    long long acc{0};

    for (const auto& line : lines) {
        if (line.empty()) {
            continue;
        }

        Row row = parse_line(line);
        // std::cout << row << "\n";

        const auto& groups  = row.groups;
        const auto& pattern = row.pattern;

        int best = min_groups_to_match(groups, pattern);

        if (best == -1) {
            // std::cout << "No matching combination for this row\n";
        } else {
            // std::cout << "Minimal groups to match: " << best << "\n";
            acc += best;
        }
    }

    return acc;
}

void dfs_presses(const Groups& groups,
                 const Values& target,
                 std::size_t gi,          // current group index
                 Values& accum,           // current accumulated counts
                 int presses,             // current total presses
                 int& best)               // best found so far
{
    // prune if already worse than best
    if (presses >= best) {
        return;
    }

    const std::size_t G = groups.size();

    // If we've assigned press counts for all groups, check if we matched target
    if (gi == G) {
        if (accum == target) {
            best = presses;
        }
        return;
    }

    const auto& group = groups[gi];

    // First, compute the max times we *could* press this group without
    // immediately exceeding any target entry.
    int max_k = 20; // heuristic cap
    for (std::uint16_t idx : group) {
        if (idx >= target.size()) {
            throw std::runtime_error("group index out of bounds in dfs_presses");
        }
        const auto remaining = static_cast<int>(target[idx]) - static_cast<int>(accum[idx]);
        if (remaining < 0) {
            // Already exceeded target in some dimension earlier; dead branch
            return;
        }
        // Each press adds +1 there, so we can press at most 'remaining' times
        if (remaining < max_k) {
            max_k = remaining;
        }
    }

    // Try k presses of this group, from 0 up to max_k
    // k = 0 means skip this group entirely
    for (int k = 0; k <= max_k; ++k) {
        if (k > 0) {
            // Apply one more press of this group (we build up incrementally)
            for (std::uint16_t idx : group) {
                accum[idx] = static_cast<std::uint16_t>(accum[idx] + 1);
            }
            ++presses;
        }

        // Recurse to next group
        dfs_presses(groups, target, gi + 1, accum, presses, best);
    }

    // Backtrack accum to original state
    for (int k = 0; k < max_k; ++k) {
        // we pressed this group k times more than original (due to the loop),
        // so subtract k from all its indices
        for (std::uint16_t idx : group) {
            accum[idx] = static_cast<std::uint16_t>(accum[idx] - 1);
        }
        --presses;
    }
}

int min_presses_to_match_values(const Groups& groups,
                                const Values& target)
{
    const std::size_t G = groups.size();

    if (G == 0) {
        // Only valid if target is all zeros
        bool all_zero = true;
        for (auto v : target) {
            if (v != 0) {
                all_zero = false;
                break;
            }
        }
        return all_zero ? 0 : -1;
    }

    // initial accum = all zeros
    Values accum(target.size(), static_cast<std::uint16_t>(0));
    int best = std::numeric_limits<int>::max();

    dfs_presses(groups, target, 0, accum, 0, best);

    if (best == std::numeric_limits<int>::max()) {
        throw std::runtime_error("No solution found - increase heuristic");
    }
    return best;
}

long long solve_part2(const std::vector<std::string> &lines) {
    long long acc{0};

    for (const auto& line : lines) {
        if (line.empty()) {
            continue;
        }

        Row row = parse_line(line);
        std::cout << row << "\n";

        const auto& groups = row.groups;
        const auto& target = row.values;  // part 2 uses values as target

        int best = min_presses_to_match_values(groups, target);

        if (best == -1) {
            std::cout << "No matching combination (within heuristic) for this row\n";
            // Depending on what you want:
            // either ignore, or throw, or treat as large cost.
        } else {
            std::cout << "Minimal presses for values: " << best << "\n";
            acc += best;
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