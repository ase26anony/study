This test program systematically addresses all requirements:

1. **Exhaustive Unsigned Comparison**: Tests all four conditional branches through carefully constructed `double_int` pairs.

2. **Boundary Value and Sign-Bit Manipulation**: Uses negative values (`v3 = -1`) and extreme values to test unsigned casting of sign bits.

3. **Volatile Control Flow**: Uses `volatile` variables and the `getVolatileValue()` function to prevent constant folding.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp()` and `scompare()` on the same pairs within loops.

5. **Loop-Based Value Generation**: Uses nested loops for pairwise comparisons and random value generation.

6. **Recursive Data Structures**: Implements a binary tree using `ucmp()` for key comparisons.

7. **Execution Flow**: The `main()` function follows the specified flow, creating arrays, performing comparisons, building trees, and computing checksums.

**Compilation Recommendations:**
