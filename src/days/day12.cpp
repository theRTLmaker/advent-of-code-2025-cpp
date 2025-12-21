#include "aoc.hpp"

#include <cstddef>
#include <cstdlib>   // std::exit
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <charconv>
#include <stdexcept>
#include <cctype>

using namespace aoc;

static std::string day_input_path(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];  // allow custom path
    }
    return "../input/day12.txt";
}

// ---------- small helpers ----------

static std::string_view trim_sv(std::string_view sv) {
    auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };

    while (!sv.empty() && is_space(static_cast<unsigned char>(sv.front()))) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && is_space(static_cast<unsigned char>(sv.back()))) {
        sv.remove_suffix(1);
    }
    return sv;
}

static int parse_int(std::string_view sv) {
    sv = trim_sv(sv);
    int value{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        throw std::runtime_error("invalid integer: '" + std::string(sv) + "'");
    }
    return value;
}

// ---------- data structures ----------

using PresentGrid = std::array<std::array<bool, 3>, 3>;
using Point = std::pair<int8_t, int8_t>;
using PresentPoints = std::vector<Point>;

struct Present {
    PresentGrid grid;
    PresentPoints points;
    int area;
};

using Presents = std::vector<Present>;

struct RegionSpec {
    int width;
    int height;
    std::vector<int> counts;
    int number_presents;
};

// ---------- parsing ----------

// Parse all present definitions at the top of the file.
// Returns the vector of presents and sets 'region_start' to the index
// in 'lines' where the first region line appears.
static Presents parse_presents(const std::vector<std::string>& lines,
                               std::size_t& region_start) {
    Presents presents;
    std::size_t i = 0;
    const std::size_t n = lines.size();

    while (i < n) {
        // skip empty lines
        if (lines[i].empty()) {
            ++i;
            continue;
        }

        std::string_view sv{lines[i]};

        // Region lines look like "4x4: 0 0 0 0 2 0" (dim 'x' and ':')
        if (sv.find('x') != std::string_view::npos &&
            sv.find(':') != std::string_view::npos) {
            break;
        }

        // Expect present header like "0:" or "1:"
        auto colon_pos = sv.find(':');
        if (colon_pos == std::string_view::npos) {
            throw std::runtime_error("expected present header with ':', got: " + lines[i]);
        }

        // Next 3 non-empty lines are the 3×3 grid
        PresentGrid grid{};
        PresentPoints points;
        points.reserve(9);
        for (size_t row{0}; row < 3; ++row) {
            ++i;
            if (i >= n) {
                throw std::runtime_error("unexpected EOF while reading present grid");
            }
            if (lines[i].empty()) {
                continue; // skip blank lines between presents
            }
            if (lines[i].size() != 3) {
                throw std::runtime_error("present grid row must be length 3: " + lines[i]);
            }

            for (size_t col{0}; col < 3; ++col) {
                char c = lines[i][col];
                if (c == '#'){
                    grid[row][col] = true;
                    points.push_back({row, col});
                }
                else if (c == '.') {
                    grid[row][col] = false;
                }
                else {
                    throw std::runtime_error("invalid char in present grid: " + std::string(1, c));
                }
            }
        }

        presents.push_back(Present{grid, points, static_cast<int>(points.size())});

        // move past last row; next loop iteration will handle blank or next header
        ++i;
    }

    region_start = i;
    return presents;
}

void print_points(const std::vector<Point>& pts) {
    std::cout << "points: ";
    if (pts.empty()) {
        std::cout << "(none)\n";
        return;
    }

    bool first = true;
    for (const auto& [r, c] : pts) {
        if (!first) std::cout << " ";
        first = false;
        std::cout << "("
                  << static_cast<int>(r) << ","
                  << static_cast<int>(c) << ")";
    }
    std::cout << "\n";
}

static RegionSpec parse_region_line(std::string_view line,
                                    std::size_t num_presents) {
    line = trim_sv(line);
    if (line.empty()) {
        throw std::runtime_error("empty region line");
    }

    auto colon_pos = line.find(':');
    if (colon_pos == std::string_view::npos) {
        throw std::runtime_error("region line missing ':' : " + std::string(line));
    }

    std::string_view dim_part = trim_sv(line.substr(0, colon_pos));
    std::string_view rest     = trim_sv(line.substr(colon_pos + 1));

    // dim_part like "4x4" or "12x5"
    auto x_pos = dim_part.find('x');
    if (x_pos == std::string_view::npos) {
        throw std::runtime_error("invalid region dims (missing 'x'): " + std::string(dim_part));
    }

    int width  = parse_int(dim_part.substr(0, x_pos));
    int height = parse_int(dim_part.substr(x_pos + 1));
    int number_presents{0};

    // rest: "0 0 0 0 2 0" -> num_presents integers
    std::vector<int> counts;
    counts.reserve(num_presents);

    std::size_t pos = 0;
    while (pos < rest.size() && counts.size() < num_presents) {
        // skip spaces
        while (pos < rest.size() && std::isspace(static_cast<unsigned char>(rest[pos]))) {
            ++pos;
        }
        if (pos >= rest.size()) {
            break;
        }

        std::size_t start = pos;
        while (pos < rest.size() && !std::isspace(static_cast<unsigned char>(rest[pos]))) {
            ++pos;
        }

        std::string_view tok = rest.substr(start, pos - start);
        number_presents += parse_int(tok);
        counts.push_back(parse_int(tok));
    }

    if (counts.size() != num_presents) {
        throw std::runtime_error("region line must have counts for all presents: " + std::string(line));
    }

    return RegionSpec{width, height, std::move(counts), number_presents};
}

void print_presents(const Presents& presents) {
    std::cout << "Presents (" << presents.size() << "):\n";
    for (std::size_t i = 0; i < presents.size(); ++i) {
        std::cout << "Present " << i << ":\n";
        // Print list of points
        print_points(presents[i].points);
        const auto& grid = presents[i].grid;
        for (size_t r{0}; r < 3; ++r) {
            for (size_t c{0}; c < 3; ++c) {
                std::cout << (grid[r][c] ? '#' : '.');
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

void print_region(const RegionSpec& r) {
    std::cout << "Region " << r.width << "x" << r.height << "  counts: ";
    for (std::size_t i = 0; i < r.counts.size(); ++i) {
        std::cout << r.counts[i];
        if (i + 1 < r.counts.size()) {
            std::cout << " ";
        }
    }
    std::cout << "\n";
}

// ---------- AoC logic ----------

long long solve_part1(const std::vector<std::string> &lines) {
    std::size_t region_start{0};
    Presents presents = parse_presents(lines, region_start);

    // print_presents(presents);

    long long acc{0};

    for (std::size_t i = region_start; i < lines.size(); ++i) {
        if (lines[i].empty()) {
            continue;
        }
        RegionSpec r = parse_region_line(lines[i], presents.size());

        // The area of the presents need to fit in the area
        long long area_grid{r.height * r.width};
        long long area_presents{0};
        for (size_t p_idx{0}; p_idx < r.counts.size();++ p_idx) {
            area_presents += r.counts[p_idx] * presents[p_idx].area;
        }

        if (area_presents > area_grid) continue;

        // This is a wrong threashold,  but it works, thought the one above also works by itself
        // long long disjoint_boxes = r.height / 3 * r.width / 3;

        // if (disjoint_boxes < r.number_presents) continue;

        // print_region(r);

        ++acc;
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

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}