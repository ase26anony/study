This test program:

1. **Exhaustively tests all four branches** of `ucmp` through pairwise comparisons of diverse `double_int` values.

2. **Manipulates sign bits** by using `INT64_MIN` (MSB set) and comparing it with positive values, ensuring the unsigned cast is properly tested.

3. **Uses volatile variables** to prevent constant folding, forcing runtime execution of the comparison logic.

4. **Mixes signed and unsigned comparisons** by calling both `ucmp` and `scompare` on the same pairs.

5. **Uses loops** to generate many comparison instances with varying relationships.

6. **Implements a binary tree** that uses `ucmp` for key comparisons, testing the method in a complex control flow context.

7. **Computes a checksum** to ensure all operations have observable effects.

To compile and run with coverage:
