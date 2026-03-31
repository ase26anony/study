## Key Design Features:

1. **Four Distinct Comparison Types**: Separate functions for `>`, `>=`, `<`, and `<=` comparisons ensure each switch case is exercised.

2. **Vectorization-Friendly Patterns**:
   - Conditional assignments (`out[i] = (a[i] op b[i]) ? a[i] : b[i]`)
   - Reduction pattern with comparisons
   - Aligned memory accesses with `__builtin_assume_aligned`
   - `restrict` keyword for alias analysis

3. **Preventing Optimizations**:
   - Loop size depends on `argc` to prevent constant propagation
   - Non-uniform data initialization using `argc` and indices
   - Volatile checksum printing to prevent dead code elimination

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Compilation Recommendations:
