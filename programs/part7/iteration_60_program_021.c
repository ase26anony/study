This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout with various comparisons and arithmetic operations.

2. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed` function and Test 6 explicitly compare signed and unsigned 128-bit values, which should trigger the `(unsigned HOST_WIDE_INT)` casts in the uncovered code.

3. **Loop-Based Range Testing**: Test 3 loops across the 64-bit boundary (from negative to positive), performing comparisons at each step where the high word might change.

4. **Function Returns**: Multiple helper functions (`cmp_int128`, `cmp_uint128`, `cmp_mixed`) return comparison results, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: Test 5 uses chained comparisons with logical operators, and Test 4 uses arithmetic operations followed by comparisons.

6. **Boundary Testing**: Test 8 specifically creates cases where:
   - High words are equal but low words differ (triggering `a.low < b.low` or `a.low > b.low`)
   - High words differ by 1 (triggering `(unsigned)a.high < (unsigned)b.high`)

To compile and run:
