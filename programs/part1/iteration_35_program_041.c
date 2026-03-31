This test program:

1. **Exactly replicates the uncovered code** in `test_fixed_value_range_condition()` function, performing the same initialization and conditional check.

2. **Explores the parameter space** with different `i_f_bits` values (0, 1, 4, 16, 31, 32, 63) to test various bit-width scenarios.

3. **Tests all conditional paths**:
   - Case 1: Triggers `a_high.sgt(max_r)` path (true)
   - Case 2: Triggers `a_high == max_r && a_low.ugt(max_s)` path (true)
   - Case 3: Makes entire condition false (negative `a_high`)
   - Case 4: Tests boundary where `a_low == max_s` (false)
   - Cases 5-6: Tests edge cases with min/max values

4. **Includes verification** of the intermediate calculations (like `max_s` after `zext`) to ensure the logic is working correctly.

5. **Uses GCC's selftest framework** with `ASSERT_*` macros for proper integration.

To compile and run this test:
