## Key Design Features:

1. **Four Distinct Comparison Types**: Each function handles one of the required operators (`>`, `>=`, `<`, `<=`).

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with aligned arrays
   - Conditional assignments and reductions
   - `restrict` keyword and `__builtin_assume_aligned` for alias analysis

3. **Preventing Optimizations**:
   - Loop bounds depend on `argc`
   - Array initialization uses `argc` for variability
   - Checksums printed at the end
   - Separate functions prevent loop fusion

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Compilation Recommendations:
