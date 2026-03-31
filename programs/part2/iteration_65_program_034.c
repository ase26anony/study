## Key Design Features:

1. **Four Distinct Comparison Types**: Each function uses one of the four operators (`>`, `>=`, `<`, `<=`) to trigger the corresponding switch cases.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with array accesses
   - Conditional assignments and reductions
   - Alignment hints using `__builtin_assume_aligned`
   - `restrict` keyword for alias analysis

3. **Preventing Optimizer Interference**:
   - Loop bounds depend on `argc`
   - Non-uniform data initialization
   - Checksum computation and volatile printing
   - Separate functions to prevent loop fusion

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Compilation Recommendations:
