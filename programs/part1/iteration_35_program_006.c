This test program:

1. **Replicates the exact uncovered block** from `fixed-value.cc` in the `test_fixed_value_range_condition` function.

2. **Tests multiple scenarios** to cover different paths through the conditional:
   - Scenario 1: Triggers `a_high.sgt(max_r)` path (true)
   - Scenario 2: Triggers `a_high == max_r && a_low.ugt(max_s)` path (true)
   - Scenarios 3-4: Make the condition false
   - Scenarios 5-6: Test edge cases with extreme values

3. **Varies `i_f_bits`** across multiple values (0, 1, 4, 16, 31, 32, 63) to test different behaviors of `zext`, `alshift`, and `sext` operations.

4. **Uses GCC's internal testing framework** (`selftest`) with `ASSERT_TRUE` and `ASSERT_FALSE` macros.

5. **Handles the double-int structure correctly** using GCC's internal `double_int` API with proper initialization and operations.

To compile and run this test:
