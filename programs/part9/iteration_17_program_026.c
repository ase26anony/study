This program creates the complex loop nesting patterns needed to trigger the uncovered bitmap intersection logic:

1. **Perfect Nesting** (`test_perfect_nesting`): Creates loops where inner loop blocks are proper subsets of outer loop blocks, testing the case where `bitmap_intersect_compl_p` returns false for both comparisons.

2. **Partial Overlap** (`test_partial_overlap`): Creates loops that share some basic blocks but each has unique blocks, testing the asymmetric containment cases.

3. **Sibling Loops** (`test_sibling_loops`): Creates loops at the same nesting level that don't share blocks, testing the `bitmap_intersect_p` false case (the `continue` path).

4. **Complex Nesting** (`test_complex_nesting`): Mixes all patterns with `goto` statements creating non-contiguous block ranges.

5. **Mixed Control Flow** (`test_mixed_control_flow`): Combines different loop types with complex control flow to stress the bitmap analysis.

**Key features that trigger the target logic:**
- Multiple `goto` statements creating non-contiguous basic blocks
- `switch` statements inside loops with fall-through cases
- Mixed loop types (`for`, `while`, `do-while`, infinite loops)
- Helper functions marked `always_inline` containing loops
- Tail recursion creating loop-like structures
- `__restrict` qualifiers and `__builtin_expect` for optimization hints
- `#pragma GCC unroll` directives
- Complex conditions with short-circuit evaluation
- Early exits via `break` and `continue` at different nesting levels

**Compilation recommendations:**
