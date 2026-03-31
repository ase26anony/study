## Key Design Features:

1. **Four Distinct Comparison Types**: 
   - `test_gt_expr`: Uses `>` operator with integers
   - `test_ge_expr`: Uses `>=` operator with floats
   - `test_lt_expr`: Uses `<` operator with doubles
   - `test_le_expr`: Uses `<=` operator with integers

2. **Vectorization-Friendly Patterns**:
   - All loops use `restrict` and `__builtin_assume_aligned`
   - Simple data-parallel operations with conditional assignments
   - Additional reduction pattern in `test_reduction_gt`

3. **Anti-Optimization Measures**:
   - Loop bounds depend on `argc` to prevent constant propagation
   - Array initialization uses varying patterns based on indices and `argc`
   - Final checksum computation ensures all results are used

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Compilation Recommendations:
