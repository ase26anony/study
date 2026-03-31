This test provides:

1. **Exact replication of the uncovered block**: The `test_fixed_value_range_condition` function mirrors the exact code from lines 264-277.

2. **Comprehensive parameter exploration**: Tests multiple `i_f_bits` values (0, 1, 4, 16, 31, 32, 63) to exercise different behaviors of `zext`, `alshift`, and `sext`.

3. **All conditional paths covered**:
   - **Scenario 1**: Forces `a_high.sgt(max_r)` to be true by using positive `a_high`
   - **Scenario 2**: Forces `a_high == max_r && a_low.ugt(max_s)` to be true
   - **Scenario 3**: Makes entire condition false with negative `a_high`
   - **Scenario 4**: Tests the boundary case where `a_high == max_r` but `a_low <= max_s`

4. **Edge cases**: Tests with maximum and minimum possible `double_int` values.

5. **Integration with GCC's selftest**: Uses the proper framework with `ASSERT_*` macros and registration function.

To compile and run this test:
