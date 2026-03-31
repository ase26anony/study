This test program is designed to:

1. **Trigger the specific comparison paths**:
   - `test_high_part_greater`: Creates values where the high part may be > 0 (triggering `a_high.sgt(max_r)`)
   - `test_zero_high_large_low`: Creates values where high part = 0 but low part is large (targeting `a_high == max_r && a_low.ugt(max_s)`)

2. **Use various value range patterns**:
   - Loop induction variables with different bounds
   - Bit manipulation operations
   - Mixed signed/unsigned comparisons
   - 128-bit arithmetic for wider range analysis
   - Template instantiations for different types

3. **Employ compiler optimization triggers**:
   - `__attribute__((noinline))` to control inlining
   - Complex loop structures with multiple exit conditions
   - Boundary value comparisons
   - Inter-procedural analysis via global variables

4. **Recommended compilation**:
