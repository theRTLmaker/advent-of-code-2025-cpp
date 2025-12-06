#include "aoc.hpp"

#include <cstdint>
#include <cstdlib>   // std::exit
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <utility>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day06.txt";
}

long long solve_part1(const std::vector<std::string> &lines) {
    std::vector<std::vector<int unsigned>> math_problems;
    long long acc{0};
    bool operator_line{false};
    for (const auto &line : lines) {
        // Find first non-space character
        auto it = std::find_if(line.begin(), line.end(),
                               [](unsigned char ch) { return !std::isspace(ch); });

        // Skip empty lines
        if (it == line.end()) {
            continue;
        }

        // If first non-space is not a digit, assume this is the operator line: 
        if (!std::isdigit(static_cast<unsigned char>(*it))) {
            // print the math problems for debugging
            // for (std::size_t col = 0; col < math_problems.size(); ++col) {
            //     std::cout << "Column " << col << ": ";
            //     for (const auto &val : math_problems[col]) {
            //         std::cout << val << " ";
            //     }
            //     std::cout << "\n";
            // }
            // for (std::size_t row = 0; row < 4; ++row) {
            //     // print the math problems for debugging
            //     for (std::size_t col = 0; col < math_problems.size(); ++col) {
            //         std::cout << math_problems[col][row] << " ";
            //     }
            //     std::cout << std::endl;
            // }
            operator_line = true;
        }

        if (operator_line) {
            // Parse numbers from this line
            std::istringstream iss(line);
            char math_op;

            auto column_it = math_problems.begin();

            while (iss >> math_op) {
                if (column_it == math_problems.end()) {
                    throw std::runtime_error("More operators than columns");
                }
                const auto &column = *column_it;
                if (column.empty()) {
                    throw std::runtime_error("Empty column in math_problems");
                }
                // Print operator for debug
                // std::cout << tmp << std::endl;
                long long math{column.front()};

                for (auto row_it = std::next(column.begin()); row_it != column.end(); ++row_it) {
                    const auto row = *row_it;
                    switch (math_op) {
                        case '*' :
                            math *= row;
                            break;
                        case '+' :
                            math += row;
                            break;
                        default:
                            throw std::runtime_error("Unsupported operator in input");
                    }
                }

                acc += math;

                ++column_it;
            }
        } else {
            // Parse numbers from this line
            std::istringstream iss(line);
            std::vector<unsigned int> row_values;
            unsigned int tmp;

            while (iss >> tmp) {
                row_values.push_back(tmp);
            }

            if (row_values.empty()) {
                continue;
            }

            if (math_problems.empty()) {
                // First numeric line: create one vector per column
                math_problems.resize(row_values.size());
            } else if (row_values.size() != math_problems.size()) {
                throw std::runtime_error("Inconsistent number of columns in input");
            }

            // Append each value to its corresponding column
            for (std::size_t col = 0; col < row_values.size(); ++col) {
                math_problems[col].push_back(static_cast<int unsigned>(row_values[col]));
            }
        }
    }
    return acc;
}

long long solve_part2(const std::vector<std::string> &lines) {
    std::vector<std::pair<char, uint8_t>> op_size;
    long long acc{0};
    for (const auto &line : lines) {
        // Find first non-space character
        auto it = std::find_if(line.begin(), line.end(),
                               [](unsigned char ch) { return !std::isspace(ch); });
                               
        // Skip empty lines
        if (it == line.end()) {
            continue;
        }
        
        // Find the line that contains the operators
        if (std::isdigit(static_cast<unsigned char>(*it))) {
            continue;
        }
        
        // Math operators are separated by spaces.
        // The number of spaces indicates the width of the biggest number in the corresponding column.
        // For example:
        // +  *   +  //
        // indicates that:
        // - the first operator is '+', and the biggest number in the first column is 2 digit wide.
        // - the second operator is '*', and the biggest number in the second column is 3 digit wide.
        // - the third operator is '+', and the biggest number in the third column is 2 digit wide.
        // So parse the line to obtain the operators and their sizes.
        std::istringstream iss(line);
        char math_op, prev_math_op;
        uint8_t size{0};
        while(iss.get(math_op)) {
            if (math_op == ' ') {
                ++size;
            } else {
                if (size > 0) {
                    op_size.emplace_back(prev_math_op, size);
                    size = 0;
                }
                prev_math_op = math_op;
            }
        }
        // Append the last one
        op_size.emplace_back(prev_math_op, size + 1); // +1 to account for the operator character itself
        
        // Print math operators and sizes for debugging
        // for (const auto &[op, sz] : op_size) {
        //     std::cout << "Operator: " << op << ", Size: " << static_cast<int>(sz) << "\n";
        // }   
    }
    
    std::vector<std::vector<std::string>> math_problems;
    // Now parse the numbers knowing the size of each column
    // So if we find spaces, and operator is +, we add 0, if operator is *, we multiply by 1
    for (const auto & line : lines) {        
        // Skip the operator line
        if (!(std::isdigit(static_cast<unsigned char>(*line.begin())) || std::isspace(static_cast<unsigned char>(*line.begin())))) {
            continue;
        }
        
        // Now parse the numbers according to op_size
        auto line_it = line.begin();
        for (size_t col = 0; col < op_size.size(); ++col) {
            auto &[op, sz] = op_size[col];
            std::string num_str;
            for (uint8_t i = 0; i < sz && line_it != line.end(); ++i, ++line_it) {
                num_str += *line_it;
            }
            
            // Store the number as a string
            std::string number = num_str;
            
            // std::cout << "Parsed value: " << number << " with operator: " << op << "\n";
            
            // Move to the next character after the current number
            ++line_it;
            
            // Store the number in math_problems
            if (math_problems.empty()) {
                // First numeric line: create one vector per column
                math_problems.resize(op_size.size());
            }

            math_problems[col].push_back(number);
            
            // std::cout << "Column " << col << ": ";
            // for (const auto &val : math_problems[col]) {
            //     std::cout << val << " ";    
            // }
            // std::cout << "\n";
        }
    }

    for (std::size_t col = 0; col < math_problems.size(); ++col) {
        const auto &[op, sz] = op_size[col];
        const auto &column = math_problems[col]; // e.g. {"12", "34", "56"}

        if (column.empty()) {
            throw std::runtime_error("Empty column in math_problems");
        }

        // Optional sanity check: all strings at least sz long
        for (const auto &token : column) {
            if (token.size() < sz) {
                throw std::runtime_error("Token shorter than expected digit size");
            }
        }

        long long math = 0;
        bool first_num = true;

        // Each column is a math problem to solve.
        // For each digit position d, build a vertical number:
        // num_d = digits column[0][d], column[1][d], ..., column[n-1][d]
        for (std::size_t d = 0; d < sz; ++d) {
            long long num = 0;

            for (const auto &token : column) {
                char ch = token[d];
                if (std::isspace(static_cast<unsigned char>(ch))) {
                    continue    ;
                }
                num = num * 10 + (ch - '0');
            }
            // std::cout << "Constructed number at digit position " << d << ": " << num << "\n";
            if (first_num) {
                math = num;
                first_num = false;
            } else {
                switch (op) {
                    case '*':
                        math *= num;
                        break;
                    case '+':
                        math += num;
                        break;
                    default:
                        throw std::runtime_error("Unsupported operator in input");
                }
            }
        }

        acc += math;
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