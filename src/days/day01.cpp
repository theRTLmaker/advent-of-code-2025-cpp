#include "aoc.hpp"

#include <cassert>
#include <cstdlib>   // std::exit
#include <iostream>
#include <string>

using namespace aoc;

namespace {
    std::string day_input_path(int argc, char **argv) {
        if (argc > 1) {
            return argv[1];  // allow custom path
        }
        return "input/day01.txt";
    }
}

int get_new_dial_pos(int current_pos, char turn, int delta) {
    delta = delta % 100; // wrap around full circle
    if (turn == 'L') { // turn left
        if (current_pos - delta < 0) {
            current_pos = 100 + (current_pos - delta);
        } else {
            current_pos = current_pos - delta;
        }
    } else if (turn == 'R') {  // turn right
        if (current_pos + delta >= 100) {
            current_pos = (current_pos + delta) - 100;
        } else {
            current_pos = current_pos + delta;
        }
    } else {
        throw std::runtime_error("Invalid turn character: " + std::string(1, turn));
    }
    return current_pos;
}

int solve_part1(const std::vector<std::string> &lines) {
    int count_zero{0};
    // Dial pos starts at 50
    int dial_pos{50};

    for (const auto &line : lines) {
        char turn = line[0];                   // 'L' or 'R'
        int value = std::stoi(line.substr(1)); // from pos 1 to end
        dial_pos = get_new_dial_pos(dial_pos, turn, value);
        if (dial_pos == 0) {
            count_zero++;
        }
        // std::cout << "turn = " << turn << ", value = " << value << ", new_pos = " << dial_pos << "\n";
    }
    return count_zero;
}

int solve_part2(const std::vector<std::string> &lines) {
    int count_zero{0};
    // Dial pos starts at 50
    int dial_pos{50};

    for (const auto &line : lines) {
        char turn = line[0];                   // 'L' or 'R'
        int value = std::stoi(line.substr(1)); // from pos 1 to end

        int factor = turn == 'R' ? 1 : -1;
        // std::cout << "count_zero = " << count_zero << ", turn = " << turn << ", value = " << value << ", dial_pos = " << dial_pos;
        if (value > 100) {
            count_zero += value / 100;
            value = value % 100;
        }

        int new_dial_pos = dial_pos + factor * value;
        // std::cout << ", new_dial_pos = " << new_dial_pos << " " << abs(new_dial_pos) % 100 << std::endl;
        if (abs(new_dial_pos) % 100 == 0 ||
            new_dial_pos > 100 ||
            // Avoid double counting when starting from zero and going negative
            (dial_pos != 0 && new_dial_pos < 0)) {
            // We will cross zero
            ++count_zero;
            // std::cout << "  -> crossed zero! count_zero = " << count_zero << std::endl;
        }

        if (new_dial_pos < 0) {
            new_dial_pos += 100;
        }

        assert(new_dial_pos >= 0 && new_dial_pos < 200);
        dial_pos = new_dial_pos % 100;
        // Assert that dial_pos is within bounds
        assert(dial_pos >= 0 && dial_pos < 100);
    }
    return count_zero;
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