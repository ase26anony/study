This test program specifically targets the uncovered lines in `fixed-value.cc` by:

1. **Replicating the exact logic** from lines 264-277 in the `test_fixed_value_range_check` function.

2. **Testing multiple scenarios** to cover all conditional paths:
   - `test_scenario_high_greater_than_max_r()`: Forces `a_high.sgt(max_r)` to be true
   - `test_scenario_low_greater_than_max_s()`: Forces `a_high == max_r && a_low.ugt(max_s)` to be true
   - `test_scenario_condition_false()`: Makes the entire condition false in multiple ways
   - `test_edge_cases()`: Tests boundary conditions

3. **Varying `i_f_bits`** across a comprehensive set of values (0, 1, 4, 16, 31, 32, 63) to exercise different behaviors of `zext`, `alshift`, and `sext` operations.

4. **Using GCC's internal types and APIs** correctly with `double_int` operations like `from_shwi`, `zext`, `alshift`, `sext`, `sgt`, `ugt`, `add`, and `sub`.

5. **Integrating with GCC's selftest framework** through the `gcc_register_test` function, which allows the test to be run when GCC is compiled with `-fself-test`.

To compile and run this test:
