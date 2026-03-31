This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with relational comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and related tests directly compare signed and unsigned 128-bit values, which should trigger the `(unsigned HOST_WIDE_INT)` casts in the uncovered code.

3. **Loop-Based Range Testing**: Two loops (one `for`, one `while`) iterate across 64-bit boundaries where the high word changes, performing comparisons at each step.

4. **Function Returns Based on Comparisons**: Multiple helper functions return `int` based on 128-bit comparisons, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: Includes compound conditionals with `&&` and `||` operators chaining multiple 128-bit comparisons.

6. **Boundary Cases**: Tests all branches:
   - High word less (`a.high < b.high`)
   - High word greater (`a.high > b.high`)
   - High word equal with low word less (`a.low < b.low`)
   - High word equal with low word greater (`a.low > b.low`)
   - Equality

To compile and run with the recommended options:
