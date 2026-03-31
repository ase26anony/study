This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and various comparisons between signed and unsigned 128-bit values should trigger the `(unsigned HOST_WIDE_INT)` casts seen in the uncovered code.

3. **Loop-Based Range Testing**: The nested loops with `i` and `j` create values that cross 64-bit boundaries, testing cases where high words are equal/different.

4. **Function Returns Based on Comparisons**: Multiple helper functions return comparison results, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: The `compare_with_overflow()` function chains multiple comparisons with logical operators.

6. **Boundary Testing**: Specific test cases target:
   - High words equal, low words different (lines 1290-1293)
   - High words different (lines 1286-1289)
   - Signed/unsigned comparisons (triggers the unsigned casts)

To compile and test with the recommended options:
