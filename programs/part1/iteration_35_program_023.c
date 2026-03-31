This test program:

1. **Replicates the exact logic** from the uncovered lines in `fixed-value.cc` in the `test_fixed_value_range_condition` function.

2. **Tests multiple scenarios**:
   - `test_fixed_value_high_greater`: Tests where `a_high.sgt(max_r)` is true
   - `test_fixed_value_low_greater`: Tests where `a_high == max_r && a_low.ugt(max_s)` is true
   - `test_fixed_value_condition_false`: Tests where the entire condition is false
   - `test_fixed_value_edge_cases`: Tests edge cases with extreme values

3. **Varies `i_f_bits`** across multiple values (0, 1, 4, 16, 31, 32, 63) to test different bit-width scenarios.

4. **Uses GCC's internal testing framework** (`selftest`) with proper assertions.

To compile and run this test:
