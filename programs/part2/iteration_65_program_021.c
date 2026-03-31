## Key Design Elements:

1. **Four Distinct Comparison Functions**: Each function (`vectorized_gt`, `vectorized_ge`, `vectorized_lt`, `vectorized_le`) uses a different comparison operator, ensuring all four switch cases are exercised.

2. **Alignment Hints**: Uses `__builtin_assume_aligned` and `alignas` to help the vectorizer recognize aligned data access patterns.

3. **Conditional Assignment Pattern**: Each loop uses `(a[i] op b[i]) ? a[i] : b[i]` which creates the conditional mask pattern that triggers the bitwise transformation logic.

4. **Non-Constant Loop Bounds**: The loop size `n` depends on command-line arguments, preventing constant propagation and dead code elimination.

5. **Non-Uniform Data Initialization**: Arrays are initialized with varying patterns based on `argc` and indices, preventing trivial comparison folding.

6. **Multiple Data Types**: Uses `int`, `float`, and `double` to test different vectorization paths.

7. **Checksum Computation**: Results are aggregated and printed, ensuring computations aren't optimized away.

## Compilation Recommendations:
