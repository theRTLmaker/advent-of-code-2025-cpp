# Advent of Code 2025 – Modern C++ Solutions

Tackling the 2025 Advent of Code puzzles as daily algorithm katas. The goal is to sharpen data structure and problem-solving skills in C++ while keeping each solution efficient—treating the challenges like a stream of LeetCode problems.

## Objectives
- Build fast, readable solutions in ISO C++23 with a focus on algorithmic rigor.
- Capture puzzle inputs and outputs in a reproducible way for future benchmarking.
- Share small, composable utilities (`include/aoc.hpp`) that keep the daily entries focused on the puzzle logic.

## Repository Layout
- `src/days/dayXX.cpp` – standalone entry point per puzzle day with `solve_part1/solve_part2` implementations.
- `src/template.cpp` – ready-to-copy scaffold for starting a new day quickly.
- `include/aoc.hpp` – helper utilities for file IO, string trimming, and lightweight timing.
- `input/dayXX.txt` – puzzle inputs stored alongside the code for reproducibility.
- `cmake/CompileOptions.cmake` – strict warning set shared across all binaries.

## Progress & Gotchas
| Day | Puzzle Focus | Key Gotcha / Insight |
| --- | --- | --- |
| 01 | Dial rotations and wrap-around counters | Treat movements modulo 100 and detect zero crossings even when deltas skip past the origin. |
| 02 | Range parsing and pattern detection | `std::from_chars` keeps parsing zero-cost; vet simple digit heuristics before heavier divisibility checks. |
| 03 | Maximizing numeric scores from digit banks | Pull digits greedily while maintaining iterator state instead of sorting copies of the bank. |
| 04 | Grid stability checks | Snapshot candidate cells each iteration so removals don’t interfere with the current pass. |
| 05 | ID range membership | Merge overlapping ranges before counting to prevent double counting and cut complexity. |

## Build & Run
Prerequisites: CMake ≥ 3.16 and a C++23 compiler (tested with both Clang and GCC).

```bash
# Configure once
cmake -S . -B build

# Build a specific day (e.g., day 05)
cmake --build build --target day05

# Run with the default input path
./build/bin/dayXX

# Or point to a custom input file
./build/bin/dayXX path/to/input.txt
```

The executables print each part's answer along with microsecond timings via `aoc::time_it` and `aoc::print_answer`.
