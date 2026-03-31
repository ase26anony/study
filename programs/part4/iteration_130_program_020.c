This test program systematically covers all requirements:

1. **Exhaustive Unsigned Comparison**: Tests all four conditional branches through pairwise comparisons in nested loops.

2. **Boundary Value and Sign-Bit Manipulation**: Uses values with MSB set (negative in signed interpretation) to test unsigned cast behavior.

3. **Volatile Control Flow**: Uses `volatile` variables and function calls to prevent constant folding.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp` and `scompare` on the same pairs.

5. **Loop-Based Value Generation**: Uses loops to generate and compare arrays of `double_int` objects.

6. **Recursive Data Structures**: Implements a binary tree using `ucmp` for comparisons.

7. **Execution Flow**: The `main()` function follows the specified flow with initialization, array comparisons, tree operations, and checksum computation.

To compile and run with coverage instrumentation:
