This test program systematically exercises all four conditional branches in the `ucmp` method:

1. **Exhaustive Comparison Cases**: The nested loops compare all pairs of test cases, covering:
   - `a.high < b.high` (unsigned): e.g., (0, 0) vs (MSB set, 0)
   - `a.high > b.high` (unsigned): e.g., (MSB set, 0) vs (0, 0)
   - `a.high == b.high`, `a.low < b.low`: e.g., (10, 1) vs (20, 1)
   - `a.high == b.high`, `a.low > b.low`: e.g., (30, -1) vs (15, -1)

2. **Sign-Bit Manipulation**: Uses values with MSB set (`v3 = -1`, `v5 = 0x8000000000000000`) to test unsigned interpretation of negative signed values.

3. **Volatile Variables**: The `get_volatile_value` function and `volatile` declarations prevent compile-time constant folding.

4. **Mixed Contexts**: Calls both `ucmp` and `scompare` on the same pairs to exercise different comparison paths.

5. **Loop-Based Generation**: Creates arrays of values and performs pairwise comparisons in loops.

6. **Tree Data Structure**: Implements a binary tree using `ucmp` for comparisons, testing the logic in a more complex control flow.

To compile and run with coverage instrumentation:
