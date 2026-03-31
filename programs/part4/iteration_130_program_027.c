This program systematically tests all aspects of the `double_int::ucmp` method:

1. **Exhaustive Comparison Cases**: Tests all four conditional branches through specific test cases (Cases 1-4).

2. **Sign Bit Manipulation**: Uses negative values for `high` parts (like `-1`) which have the MSB set, testing the unsigned cast behavior.

3. **Volatile Variables**: All source values are declared `volatile` to prevent compile-time constant folding.

4. **Mixed Signed/Unsigned Contexts**: Calls both `ucmp()` and `scompare()` on the same pairs to exercise different comparison paths.

5. **Loop-Based Generation**: Uses nested loops for pairwise array comparisons and random value generation.

6. **Tree Data Structure**: Implements a binary tree using `ucmp` for comparisons, testing the method in a more complex control flow.

7. **Observable Side Effects**: Accumulates checksums from all operations to ensure nothing is optimized away.

To compile and run with coverage instrumentation:
