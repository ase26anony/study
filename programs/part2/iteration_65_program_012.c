## Key Design Features:

1. **Four Distinct Comparison Types**: Separate functions for `GT_EXPR` (`>`), `GE_EXPR` (`>=`), `LT_EXPR` (`<`), and `LE_EXPR` (`<=`) ensure each switch case is exercised.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops with element-wise operations
   - Conditional assignments and stores that generate mask operations
   - Different data types (int, float, double) to test various vectorization paths

3. **Alignment Hints**: Uses `__builtin_assume_aligned` and `alignas` to help the vectorizer analyze memory access patterns.

4. **Non-Constant Loop Bounds**: Loop size depends on `argc` to prevent constant propagation and dead code elimination.

5. **Non-Uniform Data Initialization**: Arrays are initialized with different patterns using `argc` and loop indices to prevent trivial comparison folding.

6. **Live Results**: Checksums are computed and printed to ensure computations aren't optimized away.

## Compilation Recommendations:
