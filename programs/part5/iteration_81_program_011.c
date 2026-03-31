**Key features that target the uncovered lines:**

1. **Wide Integer Comparisons**: The `compare_int128`, `compare_uint128`, and `compare_mixed` functions explicitly compare `__int128` and `unsigned __int128` values, forcing the compiler to generate comparison logic.

2. **Mixed Signed/Unsigned Comparisons**: `compare_mixed` compares signed `__int128` with unsigned `__int128`, which should trigger the unsigned cast of high parts seen in the uncovered code.

3. **Large Constants**: `LARGE_POS`, `LARGE_NEG`, `LARGE_UNSIGNED` are defined with values that require both high and low parts to be non-zero.

4. **Arithmetic with Comparisons**: `arithmetic_comparisons` performs arithmetic operations on wide integers and then compares the results, including range checks with wide integer bounds.

5. **Control Flow Based on Comparisons**: Sorting algorithms (`bubble_sort_int128`) and binary search perform many comparisons that control program flow.

6. **Unreachable Code Path**: `unreachable_comparisons` contains comparisons that should generate code but never execute, ensuring the comparison logic is generated.

7. **Checksum Calculation**: Prevents aggressive optimization from removing the comparison code.

**Compilation recommendations:**
