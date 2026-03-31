## Key Design Features:

1. **Four Separate Comparison Loops**: Each relational operator (`>`, `>=`, `<`, `<=`) has its own loop to ensure all four uncovered cases are triggered.

2. **Vectorizable Data Types**: 
   - `short` arrays for integer comparisons (highly portable)
   - `float` arrays for floating-point comparisons (requires `-ffast-math`)

3. **Non-Constant Initialization**: Arrays are filled with pseudo-random data based on `argv` to prevent compile-time evaluation.

4. **Result Usage**: Results are combined into checksums and printed, preventing dead code elimination.

5. **Multiple Iterations**: The worker functions are called multiple times with slightly modified data to ensure the vectorization logic is exercised.

## Recommended Compilation Commands:
