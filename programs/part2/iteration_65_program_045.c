## Key Design Features:

1. **Four Distinct Comparison Functions**: Each function (`test_gt_expr`, `test_ge_expr`, `test_lt_expr`, `test_le_expr`) uses a different comparison operator, ensuring all four switch cases in the uncovered code are exercised.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with element-wise operations
   - Conditional assignments (`? :` operator) that generate mask operations
   - Additional reduction pattern for extra coverage

3. **Alignment Hints**: Uses `__builtin_assume_aligned` and `alignas` to help the vectorizer analyze memory access patterns.

4. **Non-Constant Data**: Array initialization uses `argc` and loop indices to create varying data, preventing constant folding.

5. **Live Results**: Checksums are computed and printed, preventing dead code elimination.

6. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Recommended Compilation:
