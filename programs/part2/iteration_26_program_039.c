This program combines multiple techniques to trigger the selective scheduler's debug output:

1. **Complex Loop Structures**: Nested loops with data-dependent trip counts create challenging dependency graphs for the scheduler.

2. **Mixed Instruction Types**: Combines scalar operations, SIMD intrinsics, function calls, and inline assembly with register clobbers.

3. **Control Flow Complexity**: Uses computed gotos, dense/sparse switch statements, and conditional moves.

4. **Memory Access Patterns**: Non-contiguous access, pointer arithmetic, and structure field access create varied memory operations.

5. **Compiler Pragmas**: Uses `#pragma GCC unroll` and optimization attributes to influence scheduling decisions.

6. **Prevention of Dead Code Elimination**: All computations contribute to a final checksum that's printed.

To maximize coverage of the uncovered lines, compile with:
