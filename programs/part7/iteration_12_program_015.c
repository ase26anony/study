**Key features that target the uncovered code:**

1. **All Four Comparison Types**: The code includes `>`, `>=`, `<`, `<=` comparisons in both direct and operand-swapped forms.

2. **Vectorizable Loop Structure**: 
   - Fixed iteration count (1024)
   - Contiguous memory access with alignment hints
   - Simple, side-effect-free operations
   - Results stored in arrays to prevent optimization

3. **Mixed Integer Types**: Uses `int8_t`, `int16_t`, `int32_t`, `int64_t` to stress type conversion logic.

4. **Outer Loop**: Controlled by `volatile` variable to prevent unrolling and trigger outer-loop vectorization.

5. **Non-Constant Stride**: Second loop uses `i * 2` stride for complex access patterns.

6. **Alignment Hints**: Uses `__builtin_assume_aligned` to provide alignment information.

7. **Conditional Select**: Uses comparison results in ternary operators to force mask generation.

**Compilation recommendations:**
