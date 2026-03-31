This test program systematically covers all requirements:

1. **Exhaustive Unsigned Comparison Invocation**: All four conditional branches are tested through carefully constructed pairs in `test_array`.

2. **Boundary Value and Sign-Bit Manipulation**: Uses `HOST_WIDE_INT_MIN` (MSB set) and `HOST_WIDE_INT_MAX` to test unsigned casting of negative values.

3. **Volatile Control Flow**: All source values are declared `volatile` and passed through `make_double_int_volatile()` to prevent compile-time optimization.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp()` and `scompare()` on the same pairs within loops.

5. **Loop-Based Value Generation**: Nested loops perform pairwise comparisons, plus additional random testing.

6. **Recursive Data Structures**: Implements a binary tree using `ucmp()` for key comparisons, with checksum tracking.

**Compilation Recommendations:**
