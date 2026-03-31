This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed` function and test case 3 directly compare signed and unsigned 128-bit values, which should trigger the `(unsigned HOST_WIDE_INT)` casts in the uncovered code.

3. **Loop-Based Range Testing**: Test case 4 implements a loop that crosses the 64-bit high-word boundary, forcing comparisons where the high word changes from negative to positive.

4. **Function Returns Based on Comparisons**: Multiple helper functions (`cmp_128`, `cmp_u128`, `cmp_mixed`, `cmp_with_overflow`) return comparison results, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: Test case 5 creates compound conditional expressions with multiple 128-bit comparisons chained together.

6. **Boundary Conditions**: The test cases specifically target:
   - High word less/greater comparisons (test cases 1, 2)
   - High word equal with low word less/greater (test case 2)
   - Equality cases (test case 8)
   - Overflow scenarios that might affect comparison logic (test case 6)

To compile and run:
