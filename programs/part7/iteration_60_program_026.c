**Key features that target the uncovered lines:**

1. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and its calls compare `__int128` with `unsigned __int128`, which should trigger the casting to `(unsigned HOST_WIDE_INT)` seen in lines 1286-1289.

2. **Boundary Testing**: The `test_boundary_transitions()` function and loop tests values near `2^64` where the high word changes, exercising the high-word comparison logic.

3. **Loop-Based Range Testing**: Multiple loops (for and while) iterate across 128-bit ranges, performing comparisons at each step.

4. **Complex Expressions**: The program uses logical operators (`&&`, `||`) with 128-bit comparisons and ternary operators.

5. **Arithmetic with Comparisons**: The `compare_with_arithmetic()` function performs addition before comparison, which may involve overflow handling.

6. **Function Returns Based on Comparisons**: Multiple helper functions return `int` based on 128-bit comparisons, mimicking the `double_int::cmp` method.

**Compilation recommendations:**
