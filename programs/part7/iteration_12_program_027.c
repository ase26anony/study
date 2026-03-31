## Key Design Elements:

1. **Multiple Comparison Types**: The inner loop contains all four comparison operators (`>`, `>=`, `<`, `<=`) as required.

2. **Mixed Integer Types**: Uses `int32_t`, `int16_t`, and `int8_t` arrays to stress type conversion logic.

3. **Vectorizable Patterns**:
   - Fixed iteration count (N=1024)
   - Contiguous memory access with `__restrict__` qualifiers
   - Alignment hints with `__builtin_assume_aligned`
   - Simple, side-effect-free comparisons

4. **Non-Constant Stride**: Uses `i * 2` indexing pattern for some accesses.

5. **Outer Loop**: Controlled by `volatile` variable to prevent unrolling while enabling outer-loop vectorization.

6. **Result Usage**: Stores results in arrays and uses them in conditional selects to prevent optimization.

7. **Mixed Scalar/Array Comparisons**: Compares arrays with both array elements and scalar values.

## Compilation Commands:
