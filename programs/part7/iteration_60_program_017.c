This program specifically targets the uncovered comparison logic by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with arithmetic operations and comparisons.

2. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and various comparisons between signed and unsigned 128-bit values force the compiler to handle type conversions that should trigger the casting to `(unsigned HOST_WIDE_INT)`.

3. **Loop-Based Range Testing**: Two loops (for and while) iterate through ranges that cross 64-bit boundaries, with comparisons at each step.

4. **Function Returns Based on Comparisons**: Multiple helper functions return comparison results, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: Uses compound conditionals with `&&` and `||` operators chaining multiple 128-bit comparisons.

6. **Boundary Testing**: Explicitly tests all five comparison cases (high word less/greater/equal with low word less/greater/equal).

To compile and run with the recommended options:
