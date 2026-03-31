**Key features that target the uncovered code:**

1. **All four comparison operators**: The program uses `>`, `>=`, `<`, and `<=` in the same loop body, which should trigger all four case blocks in the uncovered lines.

2. **Multiple data types**: Uses `char`, `short`, `int`, and `long` types to exercise type conversion logic around comparisons.

3. **Vectorizable loop**: Fixed iteration count (N=1024), contiguous memory access, simple side-effect-free comparisons.

4. **Results are used**: Comparison results are stored in arrays and accumulated in a checksum to prevent optimization.

5. **Outer loop**: Controlled by a `volatile` variable to potentially trigger outer-loop vectorization.

6. **Constant stride access**: Uses `i * 2` stride for more complex access patterns.

7. **Alignment hints**: Uses `__builtin_assume_aligned` and `__attribute__((aligned(64)))` for alignment information.

8. **Scalar comparisons**: Also includes comparisons with scalar values to test both array-array and array-scalar patterns.

9. **Conditional select**: Additional test using comparisons in conditional select operations.

**Compilation recommendations:**
