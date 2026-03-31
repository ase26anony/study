This program creates the necessary complex loop structures to trigger the uncovered bitmap intersection logic:

1. **Perfectly nested loops** (`test_perfect_nesting`): Inner loop blocks are proper subsets of outer loop blocks
2. **Partially overlapping loops** (`test_partial_overlap`): Loops share some blocks but each has unique blocks
3. **Sibling loops** (`test_sibling_loops`): Same nesting level with no block sharing
4. **Mixed loop types**: `for`, `while`, `do-while`, and infinite loops with conditional breaks
5. **Complex control flow**: `switch` statements, `goto` labels, multiple `continue` points
6. **Function inlining**: `ALWAYS_INLINE` functions create nested structures after inlining
7. **Compiler hints**: `__restrict`, `__builtin_expect`, `#pragma GCC unroll`

To compile for maximum coverage analysis:
