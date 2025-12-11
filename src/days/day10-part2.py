import re
import sys
from typing import List, Tuple

def parse_line(line: str) -> Tuple[List[List[int]], List[int]]:
    """
    Parse a line like:
        (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}

    Returns:
        groups: list of lists of indices (one list per ( ... ))
        target: list of ints from { ... }
    """
    line = line.strip()
    if not line:
        raise ValueError("Empty line")

    # Extract all (...) groups
    group_strs = re.findall(r"\(([^)]*)\)", line)
    groups: List[List[int]] = []
    for gs in group_strs:
        gs = gs.strip()
        if not gs:
            groups.append([])
        else:
            groups.append([int(x) for x in gs.split(",")])

    # Extract {...} target
    m = re.search(r"\{([^}]*)\}", line)
    if not m:
        raise ValueError(f"No target braces found in line: {line}")
    target = [int(x) for x in m.group(1).split(",")]

    return groups, target

import numpy as np
from scipy.optimize import milp, LinearConstraint, Bounds

def solve_line_with_scipy(groups: list[list[int]], target: list[int]) -> int:
    """
    Use SciPy milp to solve:
        minimize sum_i x_i
        subject to A x = target, x >= 0, x integer
    """

    n = len(groups)        # number of variables (buttons)
    m = len(target)        # number of equations (outputs)

    # Build A matrix: A[j, i] = 1 if j in groups[i], else 0
    A = np.zeros((m, n), dtype=float)
    for i, grp in enumerate(groups):
        for j in grp:
            if j < 0 or j >= m:
                raise ValueError(
                    f"Group index {j} out of range for target size {m}"
                )
            A[j, i] += 1.0

    b = np.array(target, dtype=float)

    # Objective: minimize sum x_i
    c = np.ones(n, dtype=float)

    # variables x_i >= 0, no explicit upper bound
    lb = np.zeros(n, dtype=float)
    ub = np.full(n, np.inf, dtype=float)
    bounds = Bounds(lb, ub)

    # Equality constraint A x = b
    constraints = LinearConstraint(A, b, b)

    # All variables are integer
    integrality = np.ones(n, dtype=int)

    res = milp(
        c=c,
        integrality=integrality,
        bounds=bounds,
        constraints=constraints,
    )

    if not res.success:
        raise RuntimeError(f"milp failed: {res.message}")

    x = res.x

    # x should already be integer-valued; guard against minor numerical noise
    x_round = np.rint(x)
    if not np.allclose(x, x_round, atol=1e-6):
        raise RuntimeError(f"Non-integer MILP solution (this is weird): {x}")

    x_int = x_round.astype(int)
    total_presses = int(x_int.sum())
    return total_presses

def solve_file(path: str) -> int:
    """
    Read all lines from file, solve each machine, sum minimal presses.
    """
    total = 0
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            groups, target = parse_line(line)
            presses = solve_line_with_scipy(groups, target)
            total += presses
    return total


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} INPUT_FILE")
        sys.exit(1)

    path = sys.argv[1]
    total = solve_file(path)
    print(total)


if __name__ == "__main__":
    main()