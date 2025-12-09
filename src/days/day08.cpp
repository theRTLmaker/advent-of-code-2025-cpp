#include "aoc.hpp"

#include <cstdlib>   // std::exit
#include <iostream>
#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <ranges>
#include <cmath>
#include <unordered_set>
#include <algorithm>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day08.txt";
}

struct Coord {
    int unsigned x;
    int unsigned y;
    int unsigned z;

    bool operator== (const Coord &other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]]
    constexpr double distance_to(const Coord &other) const noexcept {
        // use signed differences to avoid unsigned underflow
        const auto dx = static_cast<long long>(x) - static_cast<long long>(other.x);
        const auto dy = static_cast<long long>(y) - static_cast<long long>(other.y);
        const auto dz = static_cast<long long>(z) - static_cast<long long>(other.z);

        // C++17+ has std::hypot(dx, dy, dz)
        // This is not constexpr until C++26, so this function will be constexpr-ish
        // only for compilers that extend it; but the signature is fine.
        return std::hypot(std::hypot(static_cast<double>(dx),
                                     static_cast<double>(dy)),
                          static_cast<double>(dz));
    }
};

struct CoordHash {
    std::size_t operator()(const Coord &p) const noexcept {
        // simple hash combine
        std::size_t h1 = std::hash<std::size_t>{}(p.x);
        std::size_t h2 = std::hash<std::size_t>{}(p.y);
        std::size_t h3 = std::hash<std::size_t>{}(p.z);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2)) ^ (h3 + 0x9e3779b97f4a7c15ULL + (h2 << 6) + (h2 >> 2));
    }
};

using Coords = std::vector<Coord>;

int unsigned parse_ll(std::string_view sv) {
    int unsigned value{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        throw std::runtime_error("invalid integer: " + std::string(sv));
    }
    return value;
}

Coords parse_input(const std::vector<std::string> &lines) {
    Coords coords;
    coords.reserve(lines.size());

    for (const auto &line : lines) {
        if (line.empty()) {
            continue;
        }

        auto comma_split = line | std::views::split(',');

        std::array<std::string_view, 3> pieces{};
        std::size_t idx = 0;

        for (auto &&part : comma_split) {
            if (idx >= pieces.size()) {
                throw std::runtime_error("too many fields in line: " + line);
            }

            // part is a range of chars; build a string_view over it
            std::string_view sv{
                &*part.begin(),
                static_cast<std::size_t>(std::ranges::distance(part))
            };
            pieces[idx++] = sv;
        }

        if (idx != pieces.size()) {
            throw std::runtime_error("too few fields in line: " + line);
        }

        coords.push_back(Coord{
            .x = parse_ll(pieces[0]),
            .y = parse_ll(pieces[1]),
            .z = parse_ll(pieces[2]),
        });
    }

    // Print coords for debugging
    // for (const auto &coord : coords) {
    //     std::cout << "Coord: (" << coord.x << ", " << coord.y <<
    //                     ", " << coord.z << ")\n";
    // }

    return coords;
}

using CoordMatrix = std::vector<std::vector<double>>;

// The distance matrix should only be one sided; distance from A to B is same as B to A
CoordMatrix compute_distance_matrix(const Coords &coords) {
    CoordMatrix distance_matrix;
    distance_matrix.resize(coords.size());
    for (std::size_t i = 0; i < coords.size(); ++i)
    {
        distance_matrix[i].resize(coords.size());
        for (std::size_t j = 0; j < coords.size(); ++j)
        {
            double dist{-1.0};
            if (j > i) {
                dist = coords[i].distance_to(coords[j]);
            }
            distance_matrix[i][j] = dist;
        }
    }

    // Print distance matrix for debugging
    // for (const auto &row : distance_matrix) {
    //     for (const auto &val : row) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << "\n";
    // }
    return distance_matrix;
}

using Pair = std::pair<size_t, size_t>;

// Get the entries pair with the smallest distance in the matrix
// the distance must be different from -1
Pair get_smallest_distance(const CoordMatrix &matrix) {
    double smallest_distance = -1.0;
    Pair coords{-1, -1};

    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            double dist = matrix[i][j];
            if (dist != -1.0) {
                if (smallest_distance == -1.0 || dist < smallest_distance) {
                    smallest_distance = dist;
                    coords = {i, j};
                }
            }
        }
    }
    return coords;
}

using JunctionBox = std::unordered_set<Coord, CoordHash>;
using VectorJunctionBoxes = std::vector<JunctionBox>;

long long solve_part1(const std::vector<std::string> &lines) {
    Coords coords = parse_input(lines);

    // Compute the 2D matrix of distances
    CoordMatrix distance_matrix = compute_distance_matrix(coords);

    VectorJunctionBoxes junction_boxes;

    for (size_t ite{0}; ite < 10; ++ite) {
        Pair closest = get_smallest_distance(distance_matrix);
        // std::cout << "Closest pair: (" << coords[closest.first].x << ", " << coords[closest.first].y << ", " << coords[closest.first].z << ") <-> ("
        //           << coords[closest.second].x << ", " << coords[closest.second].y << ", " << coords[closest.second].z << ")\n";

        // Mark this distance as used
        distance_matrix[closest.first][closest.second] = -1.0;
        distance_matrix[closest.second][closest.first] = -1.0;

        // Find if closest.first or closest.second are already in a junction box
        bool found_box_first{false};
        bool found_box_second{false};
        for (const auto &box : junction_boxes) {
            if (box.find(coords[closest.first]) != box.end()) {
                found_box_first = true;
            }
            if (box.find(coords[closest.second]) != box.end()) {
                found_box_second = true;
            }
        }
        if (!found_box_first && !found_box_second) {
            // Create a new junction box
            JunctionBox new_box;
            new_box.insert(coords[closest.first]);
            new_box.insert(coords[closest.second]);
            junction_boxes.push_back(std::move(new_box));
        } else if (found_box_first && !found_box_second) {
            // Add second to first's box
            for (auto &box : junction_boxes) {
                if (box.find(coords[closest.first]) != box.end()) {
                    box.insert(coords[closest.second]);
                    break;
                }
            }
        } else if (!found_box_first && found_box_second) {
            // Add first to second's box
            for (auto &box : junction_boxes) {
                if (box.find(coords[closest.second]) != box.end()) {
                    box.insert(coords[closest.first]);
                    break;
                }
            }
        } else {
            // Combine the two boxes
            JunctionBox combined_box;
            for (auto it = junction_boxes.begin(); it != junction_boxes.end(); ) {
                if (it->find(coords[closest.first]) != it->end() ||
                    it->find(coords[closest.second]) != it->end()) {
                    // Add all elements to combined box
                    combined_box.insert(it->begin(), it->end());
                    // Erase this box
                    it = junction_boxes.erase(it);
                } else {
                    ++it;
                }
            }
            junction_boxes.push_back(std::move(combined_box));
        }
    }

    // Print junction boxes for debugging
    // for (size_t i = 0; i < junction_boxes.size(); ++i) {
    //     std::cout << "Junction Box " << i << ":\n";
    //     for (const auto &coord : junction_boxes[i]) {
    //         std::cout << "  (" << coord.x << ", " << coord.y
    //                   << ", " << coord.z << ")\n";
    //     }
    // }

    // Sort junction_boxes vector by the size of each set
    std::sort(junction_boxes.begin(), junction_boxes.end(),
              [](const JunctionBox &a, const JunctionBox &b) {
                  return a.size() > b.size();
              });

    return static_cast<long long>(junction_boxes[0].size()) * static_cast<long long>(junction_boxes[1].size()) * static_cast<long long>(junction_boxes[2].size());
}

long long solve_part2(const std::vector<std::string> &lines) {
    Coords coords = parse_input(lines);

    // Compute the 2D matrix of distances
    CoordMatrix distance_matrix = compute_distance_matrix(coords);

    VectorJunctionBoxes junction_boxes;

    long long result{0};

    while (true) {
        Pair closest = get_smallest_distance(distance_matrix);
        if (closest.first == static_cast<size_t>(-1) ||
            closest.second == static_cast<size_t>(-1)) {
            // No more pairs
            break;
        }
        // std::cout << "Closest pair: (" << coords[closest.first].x << ", " << coords[closest.first].y << ", " << coords[closest.first].z << ") <-> ("
        //           << coords[closest.second].x << ", " << coords[closest.second].y << ", " << coords[closest.second].z << ")\n";

        // Mark this distance as used
        distance_matrix[closest.first][closest.second] = -1.0;
        distance_matrix[closest.second][closest.first] = -1.0;

        result = coords[closest.first].x * coords[closest.second].x;

        // Find if closest.first or closest.second are already in a junction box
        bool found_box_first{false};
        bool found_box_second{false};
        for (const auto &box : junction_boxes) {
            if (box.find(coords[closest.first]) != box.end()) {
                found_box_first = true;
            }
            if (box.find(coords[closest.second]) != box.end()) {
                found_box_second = true;
            }
        }
        if (!found_box_first && !found_box_second) {
            // Create a new junction box
            JunctionBox new_box;
            new_box.insert(coords[closest.first]);
            new_box.insert(coords[closest.second]);
            junction_boxes.push_back(std::move(new_box));
        } else if (found_box_first && !found_box_second) {
            // Add second to first's box
            for (auto &box : junction_boxes) {
                if (box.find(coords[closest.first]) != box.end()) {
                    box.insert(coords[closest.second]);
                    break;
                }
            }
        } else if (!found_box_first && found_box_second) {
            // Add first to second's box
            for (auto &box : junction_boxes) {
                if (box.find(coords[closest.second]) != box.end()) {
                    box.insert(coords[closest.first]);
                    break;
                }
            }
        } else {
            // Combine the two boxes
            JunctionBox combined_box;
            for (auto it = junction_boxes.begin(); it != junction_boxes.end(); ) {
                if (it->find(coords[closest.first]) != it->end() ||
                    it->find(coords[closest.second]) != it->end()) {
                    // Add all elements to combined box
                    combined_box.insert(it->begin(), it->end());
                    // Erase this box
                    it = junction_boxes.erase(it);
                } else {
                    ++it;
                }
            }
            junction_boxes.push_back(std::move(combined_box));

        }
        if (junction_boxes.size() == 1 && junction_boxes[0].size() == coords.size()) {
            std::cout << "All coords are in one junction box; terminating.\n";
            break;
        }
    }

    // Print junction boxes for debugging
    // for (size_t i = 0; i < junction_boxes.size(); ++i) {
    //     std::cout << "Junction Box " << i << ":\n";
    //     for (const auto &coord : junction_boxes[i]) {
    //         std::cout << "  (" << coord.x << ", " << coord.y
    //                   << ", " << coord.z << ")\n";
    //     }
    // }

    return result;
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