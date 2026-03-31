This test program specifically targets the uncovered lines by:

1. **Equal High Parts with Different Low Parts**: Tests with `high = 0`, `high = -1`, and `high = 1` while varying low parts.

2. **Boundary Values for Low Part**: Tests minimal differences (0 vs 1), maximal differences (UINT64_MAX-1 vs UINT64_MAX), and MSB boundary cases.

3. **Multiple Comparison Directions**: Tests both `a < b` and `a > b` for cases where high parts are equal but low parts differ.

4. **Compiler Contexts**: Uses comparisons in loops, conditional statements, and with wide integers (`__int128`) which may trigger GCC's internal `double_int` logic.

To compile and run with coverage:
