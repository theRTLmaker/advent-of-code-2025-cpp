#include "aoc.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <ranges>
#include <charconv>
#include <algorithm>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day09.txt";
}

struct Coord {
    int unsigned x;
    int unsigned y;

    bool operator== (const Coord &other) const noexcept {
        return x == other.x && y == other.y;
    }

    [[nodiscard]]
    constexpr long area(const Coord &other) const noexcept {
        // Compute the area of the rectangle defined by this and other
        // corners. Note that the module of the difference is used to ensure
        // a positive area regardless of the order of the corners.
        const long width = static_cast<long>(std::abs(static_cast<long>(x) - static_cast<long>(other.x))) + 1;
        const long height = static_cast<long>(std::abs(static_cast<long>(y) - static_cast<long>(other.y))) + 1;
        return std::abs(width * height);
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

        std::array<std::string_view, 2> pieces{};
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
            .y = parse_ll(pieces[1])
        });
    }

    // Print coords for debugging
    // for (const auto &coord : coords) {
    //     std::cout << "" << coord.x << "," << coord.y << "\n";
    // }

    return coords;
}

long long solve_part1(const std::vector<std::string> &lines) {
    Coords coords = parse_input(lines);

    long long biggest_area{0};
    for (size_t c1{0}; c1 < coords.size(); ++c1) {
        for (size_t c2{c1}; c2 < coords.size(); ++c2) {
            long long area = coords[c1].area(coords[c2]);
            biggest_area = std::max(area, biggest_area);
            // Print area for debug
            // std::cout << "(" << coords[c1].x << "," << coords[c1].y << ")x(" << coords[c2].x << "," << coords[c2].y << ")=" << area << std::endl;
        }
    }

    return biggest_area;
}

struct Rect {
    unsigned min_x;
    unsigned max_x;
    unsigned min_y;
    unsigned max_y;
};

[[nodiscard]]
constexpr Rect make_rect(const Coord& a, const Coord& b) noexcept {
    return {
        .min_x = std::min(a.x, b.x),
        .max_x = std::max(a.x, b.x),
        .min_y = std::min(a.y, b.y),
        .max_y = std::max(a.y, b.y),
    };
}

[[nodiscard]]
constexpr bool vertical_edge_crosses_interior(const Coord& e1,
                                              const Coord& e2,
                                              const Rect& r) noexcept
{
    // assume e1.x == e2.x
    const auto x = e1.x;
    const auto e_min_y = std::min(e1.y, e2.y);
    const auto e_max_y = std::max(e1.y, e2.y);

    const bool x_inside  = (r.min_x < x && x < r.max_x);
    const bool y_overlap = !(e_max_y <= r.min_y || e_min_y >= r.max_y);
    return x_inside && y_overlap;
}

[[nodiscard]]
constexpr bool horizontal_edge_crosses_interior(const Coord& e1,
                                                const Coord& e2,
                                                const Rect& r) noexcept
{
    // assume e1.y == e2.y
    const auto y = e1.y;
    const auto e_min_x = std::min(e1.x, e2.x);
    const auto e_max_x = std::max(e1.x, e2.x);

    const bool y_inside  = (r.min_y < y && y < r.max_y);
    const bool x_overlap = !(e_max_x <= r.min_x || e_min_x >= r.max_x);
    return y_inside && x_overlap;
}


long long solve_part2(const std::vector<std::string> &lines) {
    const Coords coords = parse_input(lines);
    const size_t n = coords.size();

    long long biggest_area{0};

    for (size_t c1{0}; c1 < n; ++c1) {
        const auto &coord1 = coords[c1];

        for (size_t c2{c1}; c2 < n; ++c2) {
            const auto &coord2 = coords[c2];

            const long long area = coord1.area(coord2);
            // If the new pair creates an area bigger than the biggest
            // Check if there's no coord that lives inside that reg
            if (area > biggest_area) {
                // std::cout << "TESTING: (" << coord1.x << "," << coord1.y << ")x(" << coord2.x << "," << coord2.y << ")=" << area << std::endl;
                bool valid_area{true};

                const Rect rect = make_rect(coord1, coord2);
                for (size_t edge{0}; edge < n; ++edge) {
                    const auto &e_coord1 = coords[edge];
                    const auto &e_coord2 = coords[(edge + 1) % n];
                    // Vertical edge
                    if (e_coord1.x == e_coord2.x) {
                        if (vertical_edge_crosses_interior(e_coord1, e_coord2, rect)) {
                            valid_area = false;
                            // std::cout << "not valid" << std::endl;
                        }
                    }
                    // Horizontal edge
                    else if (e_coord1.y == e_coord2.y) {
                        if (horizontal_edge_crosses_interior(e_coord1, e_coord2, rect)) {
                            valid_area = false;
                            // std::cout << "not valid" << std::endl;
                        }
                    }
                    // Diagonal edge - invalid
                    else {
                        std::cout << "WRONG EDGE: (" << e_coord1.x << "," << e_coord1.y << ") to (" << e_coord2.x << "," << e_coord2.y << std::endl;
                        throw std::runtime_error("Invalid edge");
                    }
                }

                if (valid_area) {
                    biggest_area = std::max(area, biggest_area);
                    // std::cout << "(" << coord1.x << "," << coord1.y << ")x(" << coord2.x << "," << coord2.y << ")=" << area << std::endl;
                }
            }
        }
    }

    return biggest_area;
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