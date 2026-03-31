This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed()` function and various comparisons between signed and unsigned values should trigger the `(unsigned HOST_WIDE_INT)` casting seen in the uncovered code.

3. **Loop-Based Range Testing**: The loop from -5 to 5 with scaling by `<< 60` tests boundary conditions where the high word might change.

4. **Function Returns Based on Comparisons**: Multiple helper functions return `int` based on 128-bit comparisons, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: The `complex_compare()` function chains multiple comparisons with logical operators.

6. **Arithmetic with Overflow**: The `arithmetic_and_compare()` function performs addition/subtraction that may overflow, then compares results.

7. **Boundary Testing**: Explicit tests around the 64-bit boundary where the high word transitions.

Compile with:
