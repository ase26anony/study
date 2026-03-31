## Key Design Features:

1. **Four Separate Loops**: Each relational operator (`>`, `>=`, `<`, `<=`) gets its own loop to ensure all four cases are exercised.

2. **Non-Constant Data**: Arrays are initialized with pseudo-random values based on a seed, preventing compile-time evaluation.

3. **Result Usage**: Results are combined into checksums and printed, preventing dead code elimination.

4. **Vectorizable Types**: Uses `short` (16-bit integers) and `float` for broad architecture support.

5. **Multiple Calls**: The worker functions are called multiple times with different data patterns.

6. **Attribute Optimization**: Uses `__attribute__((optimize))` to encourage vectorization even at lower optimization levels.

## Recommended Compilation Commands:
