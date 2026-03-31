**Key design elements that target the uncovered lines:**

1. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed()` function and various comparisons between `__int128` and `unsigned __int128` should trigger the `(unsigned HOST_WIDE_INT)` casts in the uncovered code.

2. **High-Word Boundary Testing**: Test cases 2 and 8 use loops that iterate around the 64-bit boundary (`0x7FFFFFFFFFFFFFFF` to `0x8000000000000000`), forcing transitions in the high word.

3. **Explicit Branch Coverage**: Test case 5 creates specific scenarios to hit all comparison branches:
   - High word less (`case2 < case1`)
   - High word greater (`case1 > case2`)
   - High word equal, low word less (`case2 < case3`)
   - High word equal, low word greater (`case3 > case2`)
   - Equality (`case1 == case1`)

4. **Complex Expressions**: Test case 4 uses compound conditions (`&&`, `||`) with both signed and unsigned 128-bit comparisons.

5. **Arithmetic with Overflow**: Test case 3 performs addition that may overflow the 64-bit low word, requiring proper high-word handling in comparisons.

6. **Function Returns Based on Comparisons**: All helper functions return `int` based on 128-bit comparisons, mimicking the `double_int::cmp` method.

**Compilation recommendations:**
