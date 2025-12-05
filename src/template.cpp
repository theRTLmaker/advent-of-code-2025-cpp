#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>
#include <string>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day0x.txt";
}

long long solve_part1(const std::vector<std::string> &lines) {
    long long acc{0};
    for (const auto &line : lines) {
        // dummy logic as placeholder
        acc += static_cast<long long>(line.size());
    }
    return acc;
}

long long solve_part2(const std::vector<std::string> &lines) {
    long long acc{0};
    for (const auto &line : lines) {
        acc += static_cast<long long>(line.size()) * 2;
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