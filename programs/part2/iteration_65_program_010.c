## Key Design Features:

1. **Four Distinct Comparison Types**:
   - `test_gt()`: Uses `>` (GT_EXPR) with conditional assignment
   - `test_ge()`: Uses `>=` (GE_EXPR) with arithmetic in branches
   - `test_lt()`: Uses `<` (LT_EXPR) producing boolean mask
   - `test_le()`: Uses `<=` (LE_EXPR) with reduction pattern

2. **Vectorization Hints**:
   - `restrict` keyword for pointer aliasing
   - `__builtin_assume_aligned` for alignment hints
   - `alignas(64)` for stack allocations
   - Different data types (int, float, double) to test various vector units

3. **Preventing Optimizations**:
   - Loop bounds depend on `argc`
   - Non-uniform data initialization
   - Final checksum computation ensures all results are used
   - Separate functions prevent loop fusion

4. **Patterns Matching Uncovered Code**:
   - Conditional assignments (`? :` operator)
   - Mask generation (`(a[i] < b[i]) ? 1 : 0`)
   - Reduction with condition (`s += (a[i] <= b[i]) ? a[i] : b[i]`)

## Compilation Recommendations:
