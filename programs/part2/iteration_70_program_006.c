**Key aspects of this test program:**

1. **Directly replicates the uncovered code logic**: The test creates `max_r`, `max_s`, `min_r`, and `min_s` exactly as in the uncovered lines, then performs the same comparison.

2. **Two targeted test cases**:
   - **Test Case 1**: Sets `a_high` to 1 (positive), triggering the `a_high.sgt(max_r)` path.
   - **Test Case 2**: Sets `a_high` to 0 and `a_low` to 2 (greater than `max_s` after zero-extension), triggering the `(a_high == max_r && a_low.ugt(max_s))` path.

3. **Uses GCC internal types**: Works with `double_int` and uses operations like `sgt`, `ugt`, `zext`, `alshift`, and `sext`.

4. **Fixed-point mode selection**: Uses `QQmode` which should give us appropriate `i_f_bits` for the zero-extension and shift operations.

5. **Runtime variability**: Uses command-line argument to select test case, preventing constant folding.

6. **Prints diagnostic information**: Shows the values being compared to verify the logic path.

**To compile and run:**
