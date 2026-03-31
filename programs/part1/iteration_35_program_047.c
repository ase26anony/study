This test program:

1. **Replicates the exact logic** from the uncovered block in a standalone function `test_fixed_value_range_condition()`.

2. **Tests multiple scenarios** to cover all conditional paths:
   - Case 1: `a_high.sgt(max_r)` is true (makes entire condition true)
   - Case 2: `a_high == max_r && a_low.ugt(max_s)` is true (makes entire condition true)
   - Case 3: Entire condition is false (tests both `a_high < max_r` and `a_low <= max_s` cases)
   - Case 4: Edge cases with extreme `double_int` values
   - Case 5: Verifies intermediate calculations

3. **Varies `i_f_bits`** across a range of values (0, 1, 4, 8, 16, 31, 32, 63) to test different bit-width scenarios.

4. **Uses GCC's internal testing framework** with `ASSERT_TRUE` and `ASSERT_FALSE` macros.

5. **Includes necessary headers** for GCC's internal types and testing framework.

To compile and run this test:
