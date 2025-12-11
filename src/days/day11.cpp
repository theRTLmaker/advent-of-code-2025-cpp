#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day11.txt";
}

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <cctype>

struct ParsedLine {
    std::string parent;                 // the 3 chars before ':'
    std::vector<std::string> child_l;    // list of 3-char tokens after ':'
};

void print_line_input(ParsedLine entry) {
    std::cout << "node = " << entry.parent << "\n";
    std::cout << "child:\n";
    for (auto& t : entry.child_l) {
        std::cout << "  " << t << "\n";
    }
}

ParsedLine parse_line(std::string_view line) {
    // Find the colon
    auto colon_pos = line.find(':');
    if (colon_pos == std::string_view::npos) {
        throw std::runtime_error("line missing ':'");
    }
    if (colon_pos != 3) {
        throw std::runtime_error("expected exactly 3 chars before ':'");
    }

    ParsedLine result;
    result.parent = std::string(line.substr(0, 3));

    // Part after ':'
    std::string_view rest = line.substr(colon_pos + 1);

    // Trim leading spaces
    auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };

    auto l = rest.begin();
    auto r = rest.end();
    while (l != r && is_space(static_cast<unsigned char>(*l))) {
        ++l;
    }
    while (r != l && is_space(static_cast<unsigned char>(*(r - 1)))) {
        --r;
    }
    rest = std::string_view(l, static_cast<std::size_t>(r - l));

    // Split by spaces into tokens
    std::size_t pos = 0;
    while (pos < rest.size()) {
        // find next space
        auto space_pos = rest.find(' ', pos);
        std::string_view token = (space_pos == std::string_view::npos)
                               ? rest.substr(pos)
                               : rest.substr(pos, space_pos - pos);

        if (!token.empty()) {
            if (token.size() != 3) {
                throw std::runtime_error("child_l token not 3 chars: '" + std::string(token) + "'");
            }
            result.child_l.emplace_back(token);
        }

        if (space_pos == std::string_view::npos) {
            break;
        }
        pos = space_pos + 1;
    }

    return result;
}

struct Node {
    std::vector<std::string> child_l;
};

using Graph = std::unordered_map<std::string, Node>;

Graph build_graph(const std::vector<std::string> &lines) {
    Graph g;
    for (const auto &line : lines) {
        auto entry = parse_line(line);

        Node& node = g[entry.parent];  // creates or gets the Node in place
        node.child_l = std::move(entry.child_l);
    }

    return g;
}

// DFS
[[nodiscard]]
long long find_paths(const Graph &g, const std::string &parent, long long count) {
    if (parent == "out") {
        return count + 1;
    }

    for (const auto & child : g.at(parent).child_l) {
        count = find_paths(g, child, count);
    }

    return count;
}

long long solve_part1(const std::vector<std::string> &lines) {
    Graph g = build_graph(lines);

    return find_paths(g, "you", 0);
}

using Mask = std::uint8_t;

struct DfsContext {
    const Graph& g;
    std::unordered_map<std::string, int> target_index;  // name -> bit
    std::unordered_map<std::string, std::unordered_map<Mask, long long>> memo;
};

// DFS
// Assume its a DAG - direct acyclic graph
[[nodiscard]]
long long find_problematic_paths(DfsContext& ctx,
                                 const std::string& parent,
                                 Mask remaining) {
    if (parent == "out") {
        return remaining == 0 ? 1LL : 0LL;
    }

    // Memo lookup
    if (auto node_it = ctx.memo.find(parent); node_it != ctx.memo.end()) {
        auto& inner = node_it->second;
        if (auto m_it = inner.find(remaining); m_it != inner.end()) {
            return m_it->second;
        }
    }

    // If parent is one of the targets, clear its bit
    if (auto it = ctx.target_index.find(parent); it != ctx.target_index.end()) {
        int bit = it->second;
        remaining &= ~(Mask(1) << bit);
    }

    auto git = ctx.g.find(parent);
    if (git == ctx.g.end()) {
        throw std::runtime_error("Missing node in graph: " + parent);
    }

    long long count{0};
    for (const auto & child : ctx.g.at(parent).child_l) {
        count += find_problematic_paths(ctx, child, remaining);
    }

    // Store in memo
    ctx.memo[parent][remaining] = count;
    return count;
}

long long solve_part2(const std::vector<std::string> &lines) {
    Graph g = build_graph(lines);

    std::vector<std::string> targets{"dac", "fft"};

    DfsContext ctx{g, {}, {}};

    Mask initial_mask = 0;
    for (std::size_t i = 0; i < targets.size(); ++i) {
        ctx.target_index[targets[i]] = static_cast<int>(i);
        initial_mask |= (Mask(1) << i);
    }


    return find_problematic_paths(ctx, "svr", initial_mask);
}

int main(int argc, char **argv) {
    try {
        const auto input_path = day_input_path(argc, argv);
        auto lines = read_lines(input_path);

        // {
        //     auto [ans1, t1] = time_it([&] { return solve_part1(lines); });
        //     print_answer(1, ans1, t1);
        // }

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