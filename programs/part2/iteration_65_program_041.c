## Key Design Elements:

1. **Four Distinct Comparison Functions**: Each function (`test_gt_expr`, `test_ge_expr`, `test_lt_expr`, `test_le_expr`) uses one of the four ordered comparison operators, ensuring all switch cases in the uncovered code are exercised.

2. **Vectorizable Patterns**: Each loop uses:
   - Element-wise conditional assignment: `c[i] = (a[i] op b[i]) ? a[i] : b[i]`
   - Different data types (int, float, double) to test various vectorization paths
   - A reduction pattern in `test_reduction_gt` for additional coverage

3. **Alignment Hints**: 
   - `__builtin_assume_aligned` on all array pointers
   - `alignas(ALIGN)` for stack allocations
   - `restrict` qualifiers to indicate no aliasing

4. **Non-Constant Loop Bounds**: The `size` variable depends on `argc`, preventing constant propagation and dead code elimination.

5. **Non-Uniform Data Initialization**: Arrays are initialized with patterns that vary based on both loop index and `argc`, ensuring comparisons aren't trivially folded.

6. **Live Results**: Checksums are computed and printed, preventing the compiler from optimizing away the computations.

## Compilation Recommendations:
