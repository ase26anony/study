This test program systematically covers:

1. **All four conditional branches** in `ucmp`:
   - `a.high < b.high` (unsigned): Tested with `di7.ucmp(di8)`
   - `a.high > b.high` (unsigned): Tested with `di9.ucmp(di10)` and negative high values
   - `a.high == b.high && a.low < b.low`: Tested with `di11.ucmp(di12)`
   - `a.high == b.high && a.low > b.low`: Tested with `di12.ucmp(di11)`

2. **Sign-bit manipulation**: Uses negative `high` values (`-1`, `-2`) which have the MSB set, testing unsigned comparison behavior.

3. **Volatile variables**: All source values are declared `volatile` to prevent compile-time optimization.

4. **Mixed signed/unsigned contexts**: Calls both `ucmp` and `scompare` on the same pairs.

5. **Loop-based generation**: Uses nested loops to compare all pairs in `di_array`.

6. **Tree data structure**: Implements a binary tree using `ucmp` for comparisons.

To compile and run with coverage analysis:
