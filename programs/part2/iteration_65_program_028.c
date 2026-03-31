## Key Design Features:

1. **Four Separate Functions**: Each function contains a loop with one of the four comparison operators (`>`, `>=`, `<`, `<=`), ensuring all switch cases in the uncovered code are exercised.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with array accesses
   - Conditional assignments and reductions
   - Aligned memory allocations with `alignas`
   - `restrict` keyword and `__builtin_assume_aligned` hints

3. **Non-Constant Loop Bounds**: Uses `argc` to determine loop count, preventing constant propagation and dead code elimination.

4. **Non-Uniform Data Initialization**: Arrays are initialized with varying patterns using both loop indices and `argc`, ensuring comparisons aren't trivially folded.

5. **Live Results**: Computes and prints a checksum, preventing the compiler from optimizing away the computations.

## Compilation Recommendations:
