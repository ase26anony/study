This test program:

1. **Replicates the exact uncovered block** in `test_fixed_value_range_condition()` function, including all the initialization and the conditional check.

2. **Tests all three required scenarios**:
   - **Scenario A**: Forces `a_high.sgt(max_r)` to be true by using positive `a_high` values
   - **Scenario B**: Forces `a_high == max_r && a_low.ugt(max_s)` to be true by setting `a_high` to 0 and `a_low` greater than the `zext`-masked `max_s`
   - **Scenario C**: Makes the entire condition false using negative `a_high` or `a_low <= max_s`

3. **Varies `i_f_bits`** across multiple values (0, 1, 4, 16, 31, 32, 63) to test different bit-width scenarios.

4. **Tests edge cases** with maximum/minimum `double_int` values and zero values.

5. **Integrates with GCC's selftest framework** through the `gcc_register_test()` function.

To compile and run this test:
