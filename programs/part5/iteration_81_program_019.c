This program specifically targets the uncovered lines in `double_int::cmp` by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` throughout, with explicit comparisons in sorting, searching, and range checking functions.

2. **Mixed Signed/Unsigned Comparisons**: 
   - Compares signed `__int128` values where high words can be negative
   - Uses `unsigned __int128` comparisons in `compare_uint128`
   - Includes explicit casts like `(unsigned __int128)a > (unsigned __int128)b`

3. **Constant Wide Integers**: Defines large constants like `((__int128)1 << 80)` and `((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64)` that have non-zero high parts.

4. **Arithmetic Result Comparisons**: Performs addition, subtraction, and shifting on wide integers, then compares the results in `test_arithmetic_comparisons()`.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if` statements, loop conditions (binary search), and sorting algorithms.

6. **Unreachable Code Path**: The `unreachable_path()` function contains a comparison that's always false but ensures the comparison logic is generated.

The execution flow initializes an array with mixed values, sorts it (triggering many comparisons), performs searches and range checks, and outputs a checksum to verify execution while preventing dead code elimination.

**Compilation recommendations:**
