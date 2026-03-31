This test program:

1. **Exhaustively tests all ucmp branches** through carefully constructed test cases
2. **Manipulates sign bits** by using negative values for high parts (case2_a with high = -1, case6_a with MSB set)
3. **Uses volatile variables** to prevent compile-time optimization
4. **Mixes signed and unsigned comparisons** by calling both `ucmp()` and `scompare()` on the same pairs
5. **Uses loops** for pairwise comparisons across arrays of values
6. **Implements a binary tree** that uses `ucmp()` for key comparisons
7. **Generates a checksum** to ensure all operations have observable effects

To compile and run with coverage instrumentation:
