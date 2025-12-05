#include "aoc.hpp"

#include <algorithm>
#include <cstdlib>   // std::exit
#include <iostream>
#include <stdexcept>
#include <string>
#include <cassert>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day03.txt";
}

long long solve_part1(const std::vector<std::string> &lines) {
    long long joltage = 0;
    for (const auto &line : lines) {
        std::vector<int> bank;
        bank.reserve(line.size());

        for (char c : line) {
            if (c < '0' || c > '9') {
                throw std::runtime_error("Invalid Character");
            }
            bank.push_back(c - '0');
        }

        // Perform 2 passes over the vector, to find the biggest element
        auto max_it = std::max_element(bank.begin(), bank.end() - 1);
        auto second_max_it = std::max_element(max_it + 1, bank.end());
        // std::cout << "Line: " << line << " Max: " << *max_it << " Second Max: " << *second_max_it << "\n";
        joltage += *max_it * 10 + *second_max_it;
    }
    return joltage;
}

long long solve_part2(const std::vector<std::string> &lines) {
    long long joltage = 0;
    for (const auto &line : lines) {
        std::vector<int> bank;
        bank.reserve(line.size());

        for (char c : line) {
            if (c < '0' || c > '9') {
                throw std::runtime_error("Invalid Character");
            }
            bank.push_back(c - '0');
        }

        long long bank_joltage{0};
        auto max_it = bank.begin();
        for (int i{11}; i >= 0; --i) {
            max_it = std::max_element(max_it, bank.end() - i);
            bank_joltage = bank_joltage * 10 + *max_it;
            // std::cout << "  Step " << (12 - i) << ": " << *max_it << " bank_joltage: " << bank_joltage << "\n";
            ++max_it;
        }
        std::cout << "Line: " << line << " bank_joltage: " << bank_joltage << "\n";
        assert (joltage + bank_joltage > joltage);
        joltage += bank_joltage;
    }
    return joltage;
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