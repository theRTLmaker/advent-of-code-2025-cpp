#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>
#include <string>
#include <utility>
#include <unordered_set>
#include <vector>
#include <unordered_map>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day07.txt";
}

std::pair<std::vector<std::vector<char>>, std::pair<size_t, size_t>> parse_grid(const std::vector<std::string> &lines) {
    std::vector<std::vector<char>> grid;
    std::pair<size_t, size_t> s_coords{-1, -1};

    for (const auto &line : lines) {
        grid.emplace_back(line.begin(), line.end());
    }

    // find 'S' coordinates
    for (size_t r = 0; r < lines.size(); ++r) {
        for (size_t c = 0; c < lines[r].size(); ++c) {
            if (lines[r][c] == 'S') {
                s_coords = {static_cast<int>(r), static_cast<int>(c)};
            }
        }
    }
    return {grid, s_coords};
}

void print_grid(const std::vector<std::vector<char>> &grid) {
    for (const auto &row : grid) {
        for (const auto &ch : row) {
            std::cout << ch;
        }
        std::cout << "\n";
    }
}

long explore_grid_part_1(std::vector<std::vector<char>> &grid, size_t r, size_t c, long split_count) {
    // Check bounds
    if (r >= grid.size() || c >= grid[0].size() || r < 0 || c < 0) {
        return split_count;
    }

    if (grid[r][c] == '^') {
        ++split_count;
        // Split the ray when finding a ^
        split_count = explore_grid_part_1(grid, r, c + 1, split_count);
        split_count = explore_grid_part_1(grid, r, c - 1, split_count);
    } else if (grid[r][c] != '|') {
        if (grid[r][c] != 'S') {
            grid[r][c] = '|';
        }
        split_count = explore_grid_part_1(grid, r + 1, c, split_count);
    }
    return split_count;
}

long solve_part1(const std::vector<std::string> &lines) {
    long split_count{0};
    auto [grid, s_coords] = parse_grid(lines);

    // Print the grid for debugging
    // print_grid(grid);
    // std::cout << "S coordinates: (" << s_coords.first << ", " << s_coords.second << ")\n";

    // Execute the grid exploration from 'S'
    split_count = explore_grid_part_1(grid, s_coords.first, s_coords.second, split_count);

    // print_grid(grid);

    return split_count;
}

using Grid = std::vector<std::vector<char>>;
struct Coord {
    size_t r;
    size_t c;

    bool operator==(const Coord &other) const noexcept {
        return r == other.r && c == other.c;
    }
};
struct CoordHash {
    std::size_t operator()(const Coord &p) const noexcept {
        // simple hash combine
        std::size_t h1 = std::hash<std::size_t>{}(p.r);
        std::size_t h2 = std::hash<std::size_t>{}(p.c);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

using Cache = std::unordered_map<Coord, std::uint64_t, CoordHash>;

std::uint64_t explore_grid_part_2(
    const Grid &grid, size_t r, size_t c,
    Cache &cache) {
    // Check bounds
    if (r >= grid.size() || c >= grid[0].size()) {
        return 1;
    }

    Coord key{r, c};
    if (auto it = cache.find(key); it != cache.end()) {
        return it->second;
    }

    std::uint64_t result{0};

    if (grid[r][c] == '^') {
        // Split the ray when finding a ^
        result += explore_grid_part_2(grid, r, c + 1, cache);
        if (c > 0) result += explore_grid_part_2(grid, r, c - 1, cache);
        else ++result;
    } else {
        result = explore_grid_part_2(grid, r + 1, c, cache);
    }
    cache[key] = result;
    return result;
}

long long solve_part2(const std::vector<std::string> &lines) {
    auto [grid, s_coords] = parse_grid(lines);

    Cache cache;
    // Execute the grid exploration from 'S'
    std::uint64_t number_of_paths = explore_grid_part_2(grid, s_coords.first, s_coords.second, cache);

    return static_cast<long long>(number_of_paths);
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