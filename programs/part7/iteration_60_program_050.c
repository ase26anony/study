This program specifically targets the uncovered lines by:

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout, with comparisons in various contexts.

2. **Mixed Signed/Unsigned Contexts**: The `cmp_mixed()` function and various comparisons between signed and unsigned 128-bit values force the compiler to handle type conversions that should trigger the casting to `(unsigned HOST_WIDE_INT)`.

3. **Loop-Based Range Testing**: Two nested loops in Test 4 iterate around values that cross 64-bit boundaries, and Test 9 uses a while loop with 128-bit comparisons.

4. **Function Returns Based on Comparisons**: Multiple helper functions (`cmp_128`, `cmp_u128`, `cmp_mixed`, `complex_compare`) return `int` results based on 128-bit comparisons, mimicking the `double_int::cmp` method.

5. **Complex Expressions**: The `complex_compare()` function chains multiple comparisons with logical operators, and Test 6 includes compound conditions with arithmetic operations.

6. **Boundary Testing**: `test_boundary_transitions()` specifically tests values near where the high word changes (around 2^63 and crossing zero).

7. **Aggregation**: All comparison results are aggregated into `total_results` to prevent dead code elimination.

**Compilation recommendations**:
