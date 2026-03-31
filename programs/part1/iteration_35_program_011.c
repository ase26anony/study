**Key aspects of this solution:**

1. **Exact Replication**: The `test_fixed_value_range_condition` function replicates the exact logic from lines 264-277 of `fixed-value.cc`, including all the initialization steps and the conditional check.

2. **Three Test Scenarios**:
   - **Scenario A**: Forces the `a_high.sgt(max_r)` path to be true by using `a_high = 1` and `max_r = 0`
   - **Scenario B**: Forces the `a_low.ugt(max_s)` path to be true by setting `a_high = max_r = 0` and making `a_low > max_s`
   - **Scenario C**: Makes the entire condition false using negative `a_high` values or `a_low <= max_s`

3. **Parameter Space Exploration**: Tests multiple `i_f_bits` values (0, 1, 4, 8, 16, 31, 32, 48, 63) to exercise different behaviors of the `zext`, `alshift`, and `sext` operations.

4. **Edge Cases**: Tests with extreme values (`double_int::max_value()` and `double_int::min_value()`) and boundary conditions like `i_f_bits = 0`.

5. **GCC Integration**: Uses GCC's internal headers and follows the selftest framework pattern. The test function `test_fixed_value_range()` can be called when GCC runs its self-tests with the `-fself-test` flag.

**Compilation and Execution**:
