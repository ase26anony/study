## Key Design Features:

1. **Four Separate Functions**: Each function contains a loop with one of the four comparison operators (`>`, `>=`, `<`, `<=`), ensuring all switch cases in the uncovered code are exercised.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with element-wise operations
   - Conditional assignments and mask storage
   - Aligned memory allocations with `__builtin_assume_aligned`
   - `restrict` keyword to indicate no aliasing

3. **Prevention of Optimizer Interference**:
   - `volatile` size variables to prevent constant propagation
   - Command-line argument (`argc`) to vary initialization
   - Checksum computation to keep results live
   - Separate functions to prevent loop fusion

4. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

## Compilation Recommendations:
