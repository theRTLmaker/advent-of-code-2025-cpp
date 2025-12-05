#include "aoc.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>   // std::exit
#include <iostream>
#include <stdexcept>
#include <string>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day04.txt";
}

std::vector<std::vector<uint8_t>> load_map(const std::vector<std::string> &lines) {
    std::vector<std::vector<uint8_t>> map;
    for (const auto &line : lines) {
        std::vector<uint8_t> row;
        for (const auto &ch : line) {
            if (ch == '.') {
                row.push_back(static_cast<uint8_t>(0));
            } else if (ch == '@') {
                row.push_back(static_cast<uint8_t>(1));
            } else {
                throw std::runtime_error("Invalid char");
            }
        }
        map.push_back(row);
    }
    return map;
}

bool valid_position(std::vector<std::vector<uint8_t>> map, size_t x, size_t y) {
    uint8_t count{0U};
    // Check up, down, left, right
    // and the diagonals
    if (y > 0) {
        count += map[y - 1][x];  // up
    }
    if (y + 1 < map.size()) {
        count += map[y + 1][x];  // down
    }
    if (x > 0) {
        count += map[y][x - 1];  // left
    }
    if (x + 1 < map[0].size()) {
        count += map[y][x + 1];  // right
    }
    if (y > 0 && x > 0) {
        count += map[y - 1][x - 1];  // up-left
    }
    if (y > 0 && x + 1 < map[0].size()) {
        count += map[y - 1][x + 1];  // up-right
    }
    if (y + 1 < map.size() && x > 0) {
        count += map[y + 1][x - 1];  // down-left
    }
    if (y + 1 < map.size() && x + 1 < map[0].size()) {
        count += map[y + 1][x + 1];  // down-right
    }
    return count < 4;
}

int unsigned solve_part1(const std::vector<std::string> &lines) {
    int unsigned rolls{0};
    auto map = load_map(lines);
    // print map for debugging
    // for (const auto &row : map) {
    //     for (const auto &cell : row) {
    //         std::cout << static_cast<int>(cell);
    //     }
    //     std::cout << "\n";
    // }

    for (size_t y{0}; y < map.size(); ++y) {
        for (size_t x{0}; x < map[0].size(); ++x) {
            if (map[y][x] == 1) {
                if (valid_position(map, x, y)) {
                    ++rolls;
                }
            }
        }
    }

    return rolls;
}

long long solve_part2(const std::vector<std::string> &lines) {
    auto map = load_map(lines);
    // print map for debugging
    // for (const auto &row : map) {
        //     for (const auto &cell : row) {
            //         std::cout << static_cast<int>(cell);
            //     }
            //     std::cout << "\n";
            // }

    long long total_rolls{0};
    int unsigned rolls{0};
    do {
        // Reset the roll count
        rolls = 0;
        std::vector<std::pair<uint8_t, uint8_t>> rolls_found;
        for (size_t y{0}; y < map.size(); ++y) {
            for (size_t x{0}; x < map[0].size(); ++x) {
                if (map[y][x] == 1) {
                    if (valid_position(map, x, y)) {
                        ++rolls;
                        rolls_found.push_back({static_cast<uint8_t>(x), static_cast<uint8_t>(y)});
                    }
                }
            }
        }
        total_rolls += rolls;
        for (const auto &[x, y] : rolls_found) {
            map[y][x] = 0;
        }
    } while (rolls > 0);

    return total_rolls;
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