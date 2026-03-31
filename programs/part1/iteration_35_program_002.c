This test program provides comprehensive coverage of the uncovered lines in `fixed-value.cc`:

## Key Features:

1. **Exact Logic Replication**: The `test_fixed_value_range_condition` function replicates the exact initialization and conditional check from lines 264-277 of `fixed-value.cc`.

2. **Four Test Scenarios**:
   - **Scenario 1**: Tests the `a_high.sgt(max_r)` path being true
   - **Scenario 2**: Tests the `a_low.ugt(max_s)` path being true
   - **Scenario 3**: Tests the condition being false
   - **Scenario 4**: Tests edge cases and boundary conditions

3. **Parameter Space Exploration**: Tests with various `i_f_bits` values (0, 1, 16, 31, 32, 63) to exercise different behaviors of `zext`, `alshift`, and `sext` operations.

4. **GCC Self-Test Integration**: Uses GCC's `selftest` framework with proper registration.

## Compilation and Execution:

To compile and run this test:
