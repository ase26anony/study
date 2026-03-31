This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Perfect Nesting**: `test_perfect_nesting` creates inner loops whose blocks are proper subsets of outer loop blocks.

2. **Partial Overlap**: `test_partial_overlap` creates loops that share some blocks but each has unique blocks.

3. **Sibling Loops**: `test_sibling_loops` creates loops at the same nesting level within a common outer loop.

4. **Multiple Entry Points**: `test_multiple_entry_loops` uses `goto` to create non-contiguous block ranges.

5. **Mixed Loop Types**: Uses `for`, `while`, `do-while`, and infinite loops with various control flow modifications.

6. **Function Inlining**: All helper functions are marked with `always_inline`.

7. **Compiler Hooks**: Uses `__builtin_expect`, `__restrict`, `#pragma GCC unroll`, and various optimization-friendly patterns.

**Compilation recommendations:**
