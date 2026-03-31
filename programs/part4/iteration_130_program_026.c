This test program systematically covers all requirements:

1. **Exhaustive Unsigned Comparison Invocation**: Tests all four conditional branches through `case1_a` to `case4_b` and the nested loops.

2. **Boundary Value and Sign-Bit Manipulation**: Uses `neg_high1`/`neg_high2` (with MSB set) and `pos_high1`/`pos_high2` to test unsigned comparison when high parts have different sign bits.

3. **Volatile Control Flow**: Uses `getVolatileValue()` function and volatile variables to prevent compile-time constant folding.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp()` and `scompare()` on the same pairs within loops.

5. **Loop-Based Value Generation**: Creates an array of `double_int` objects and performs pairwise comparisons in nested loops.

6. **Recursive Data Structures**: Implements a binary tree with `double_int` keys using `ucmp()` for comparisons during insertion.

**Compilation recommendations**:
