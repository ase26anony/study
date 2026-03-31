This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed()` function and various inline comparisons directly compare signed and unsigned 128-bit values, which should trigger the casting to `(unsigned HOST_WIDE_INT)` seen in the uncovered code.

3. **Loop-Based Range Testing**: The `test_boundary()` function loops through values near 64-bit boundaries, and `main()` calls it with ranges that cross these boundaries.

4. **Function Returns Based on Comparisons**: Multiple helper functions return `int` results based on 128-bit comparisons, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: The `complex_cmp()` function chains multiple comparisons with logical operators, and the program includes various compound conditions.

6. **Boundary Cases**: Tests values at exact 64-bit boundaries (`1 << 63`), maximum/minimum values, and values with non-zero high words.

To compile and run:
