## Key Design Features:

1. **Four Distinct Comparison Functions**: Each function (`test_gt_expr`, `test_ge_expr`, `test_lt_expr`, `test_le_expr`) targets one specific comparison operator, ensuring all four switch cases are exercised.

2. **Vectorization-Friendly Patterns**:
   - Uses `restrict` keyword to indicate no pointer aliasing
   - Employs `__builtin_assume_aligned` for alignment hints
   - Uses conditional assignment patterns (`? :`) that generate masks

3. **Preventing Optimizations**:
   - Loop bounds depend on `argc` to prevent constant propagation
   - Arrays initialized with varying, non-trivial data
   - Results aggregated into checksums that are printed

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

5. **Reduction Patterns**: Additional functions with reduction operations (`test_gt_reduction`, `test_ge_reduction`) increase the likelihood of hitting the target code.

## Compilation Recommendations:
