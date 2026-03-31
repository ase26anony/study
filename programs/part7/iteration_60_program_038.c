**Key features that target the uncovered lines:**

1. **Mixed Signed/Unsigned Comparisons**: The `cmp_mixed()` function directly compares `__int128` with `unsigned __int128`, which should trigger the casting to `(unsigned HOST_WIDE_INT)` seen in lines 1285-1293.

2. **Boundary Testing**: The loop tests values around `2^63` (where the high word changes), and `test_boundary_transitions()` explicitly tests near 64-bit boundaries.

3. **Complex Expressions**: The `complex_compare()` function uses compound conditions with `&&` and `||` operators, forcing multiple comparison evaluations.

4. **Arithmetic Operations**: The code performs 128-bit arithmetic (multiplication, subtraction) and then compares the results, testing overflow/underflow scenarios.

5. **Loop-Based Testing**: The `for` loop iterates around the critical boundary where the high word of a 128-bit integer changes.

6. **Multiple Comparison Types**: The code uses all relational operators (`<`, `>`, `<=`, `>=`, `==`, `!=`) in various contexts.

**Compilation recommendations:**
