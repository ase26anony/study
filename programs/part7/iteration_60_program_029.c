**Key features that target the uncovered lines:**

1. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and mixed comparisons in loops directly compare `__int128` with `unsigned __int128`, which should trigger the `(unsigned HOST_WIDE_INT)` casting seen in the uncovered code.

2. **High-Word Boundary Testing**: Test cases 1-5 explicitly test all branches:
   - High word less (negative vs positive)
   - High word greater (positive vs negative)
   - High word equal, low word less
   - High word equal, low word greater
   - Equality

3. **Loop-Based Range Testing**: The nested loops incrementally change 128-bit values and perform comparisons at each step, testing boundary conditions near 64-bit transitions.

4. **Complex Expressions**: The loop contains compound conditionals like `if ((a < b) && (b < c))` and `if ((a > 0) || (b < 0))` with 128-bit operands.

5. **Overflow Scenarios**: The `compare_with_overflow()` function and overflow tests perform arithmetic that may overflow, followed by comparisons.

6. **Function Returns**: Helper functions return comparison results as `int`, mimicking the `double_int::cmp` method.

**Compilation recommendations:**
