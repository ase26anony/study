## Key Design Features:

1. **Four Separate Functions**: Each function targets one comparison operator (`>`, `>=`, `<`, `<=`) to ensure all switch cases are exercised.

2. **Vectorization-Friendly Patterns**:
   - `test_gt_expr`: Uses `>` with conditional assignment
   - `test_ge_expr`: Uses `>=` with reduction pattern
   - `test_lt_expr`: Uses `<` with conditional assignment  
   - `test_le_expr`: Uses `<=` with boolean mask generation

3. **Alignment Hints**: Uses `__builtin_assume_aligned` and `alignas(64)` to help the vectorizer.

4. **Non-Constant Data**: Initialization depends on `argc` and loop indices to prevent compile-time folding.

5. **Live Results**: Computes and prints a checksum to ensure all computations are retained.

## Compilation Recommendations:
