This test program:

1. **Exhaustively tests all comparison branches** by creating specific test cases for each conditional in the `ucmp` method.

2. **Manipulates sign bits** by using high values like `0x8000000000000000` (which is negative in signed interpretation but large in unsigned) and `0x7FFFFFFFFFFFFFFF` (largest positive signed).

3. **Uses volatile variables** and function calls to prevent constant folding, ensuring runtime execution of the comparison logic.

4. **Mixes signed and unsigned comparisons** by calling both `ucmp` and `scompare` on the same pairs of values.

5. **Uses loops** to generate arrays of values and perform pairwise comparisons, creating many dynamic instances.

6. **Implements a binary tree** with `double_int` keys that uses `ucmp` for insertion, testing the comparison in a more complex control flow context.

7. **Computes and outputs a checksum** to ensure all operations have observable effects and prevent optimization removal.

To compile and run with coverage instrumentation:
