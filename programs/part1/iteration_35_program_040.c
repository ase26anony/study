This test program:

1. **Exactly replicates the uncovered logic** from lines 264-277 of fixed-value.cc in the `test_fixed_value_range_check` function.

2. **Tests multiple scenarios** for each `i_f_bits` value:
   - **Case A**: `a_high.sgt(max_r)` is true (a_high positive while max_r is 0)
   - **Case B**: `a_high == max_r && a_low.ugt(max_s)` is true (a_high is 0, a_low > max_s after zext)
   - **Case C**: Entire condition is false (a_high negative OR a_low <= max_s)

3. **Uses diverse `i_f_bits` values**: 0, 1, 16, 31, 32, 63 to test different bit-width scenarios.

4. **Includes edge cases**:
   - Maximum and minimum double_int values
   - Boundary conditions where a_low equals max_s
   - Special case for i_f_bits = 0
   - Large i_f_bits values near the double_int width limit

5. **Integrates with GCC's selftest framework** using ASSERT_TRUE macros for verification.

To compile and run this test as part of GCC's self-test suite:
