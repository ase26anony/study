This test program systematically covers all requirements:

1. **Exhaustive Unsigned Comparison Invocation**: Tests all 4 conditional branches through specific test cases
2. **Boundary Value and Sign-Bit Manipulation**: Uses negative high values where the sign bit becomes part of unsigned magnitude
3. **Volatile Control Flow**: Uses `volatile` variables for source values to prevent compile-time folding
4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp()` and `scompare()` on the same pairs
5. **Loop-Based Value Generation**: Nested loops perform pairwise comparisons between all test cases
6. **Recursive Data Structures**: Implements a binary tree using `ucmp` for key comparisons
7. **Execution Flow**: Main function follows the specified flow with checksum accumulation

To compile and run with coverage instrumentation:
