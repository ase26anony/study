This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - Perfect nesting in `test_perfect_nesting()` (inner loops fully contained)
   - Partial overlap in `test_partial_overlap()` (shared blocks via `goto`)
   - Sibling loops in `test_sibling_loops()` (same outer loop, no direct block sharing)
   - Multiple entry/exit points using `goto`, `break`, and `continue`

2. **Control Flow Graph Complexity**:
   - `if-else` statements with divergent paths
   - `switch` statements with multiple cases
   - `goto` jumps creating non-contiguous blocks
   - Multiple `continue` points in loops

3. **Mixed Loop Types**:
   - `for` loops with variable increments (`i += (i % 5) + 1`)
   - `while` loops with complex conditions
   - `do-while` loops with early exits
   - Infinite loops (`for(;;)`) with conditional breaks
   - Recursive function simulating loops

4. **Function Inlining Boundaries**:
   - `FORCE_INLINE` macro forces inlining
   - Recursive function creates loop-like CFG
   - Helper functions called from within loops

5. **Compiler Optimization Hooks**:
   - `__restrict` qualifiers on pointers
   - `__builtin_expect` for branch prediction
   - `#pragma GCC unroll` directive
   - `volatile` variables to prevent optimization
   - Array accesses with varying strides

**Compilation recommendations**:
