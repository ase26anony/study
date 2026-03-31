**Key Design Elements:**

1. **Targeting `a_high == max_r && a_low.ugt(max_s)`**:
   - `create_large_low_part()` generates values where high bits are 0 but low part exceeds `(1<<bits)-1`
   - `test_high_zero_large_low()` uses these values in comparisons that should trigger the specific path

2. **Targeting `a_high.sgt(max_r)`**:
   - `test_high_positive()` uses 128-bit arithmetic to create values with positive high parts
   - Multiplications that overflow 64-bit boundaries create non-zero high parts

3. **Range Analysis Triggers**:
   - Loops with power-of-2 limits (`test_index_at_max()`)
   - Bitwise operations that preserve range properties
   - Comparisons near boundaries

4. **Compiler Optimization Context**:
   - `__attribute__((noinline))` prevents inlining from simplifying too much
   - `__attribute__((const))` allows aggressive propagation for some functions
   - `__builtin_expect` hints at branch likelihood
   - `volatile` prevents dead code elimination

5. **Multiple Bit Widths**:
   - Tests with 32, 48, and 60 bits to match different `i_f_bits` values
   - Uses both signed and unsigned comparisons

**Compilation Recommendations:**
