This program systematically covers all requirements:

1. **Exhaustive Unsigned Comparison**: The nested loops perform `ucmp` on all pairs, exercising all four conditional branches.

2. **Boundary Value and Sign-Bit Manipulation**: Uses `INT64_MIN`, `INT64_MAX`, `-1`, `-2` to test sign-bit behavior when cast to unsigned.

3. **Volatile Control Flow**: All source values are declared `volatile` to prevent compile-time evaluation.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp` and `scompare` on the same pairs.

5. **Loop-Based Value Generation**: Creates arrays from combinations of high/low values and performs pairwise comparisons.

6. **Recursive Data Structures**: Implements a binary tree with insertion based on `ucmp` results.

**Compilation recommendations**:
