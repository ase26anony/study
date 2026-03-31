This test program:

1. **Exhaustively tests all comparison branches** through the pairwise comparisons in the nested loops and explicit test cases.

2. **Manipulates sign bits** by using values with the MSB set (like `v3` = -1 = 0xFFFFFFFFFFFFFFFF and `v5` = 0x8000000000000000), ensuring the unsigned cast is properly tested.

3. **Uses volatile variables** in `get_volatile_value()` to prevent compile-time constant folding.

4. **Mixes signed and unsigned comparisons** by calling both `ucmp()` and `scompare()` on the same pairs.

5. **Uses loop-based generation** with an array of values and performs all pairwise comparisons.

6. **Implements a tree data structure** with insertion based on `ucmp()` results.

7. **Computes a checksum** to ensure all operations have observable effects.

To compile and run with coverage instrumentation:
