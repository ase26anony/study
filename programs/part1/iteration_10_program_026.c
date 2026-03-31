## Key Design Elements:

1. **Overlapping Loop Blocks**: The `generate_loops` function creates two inner loops that both can reach the same shared handler block (cases 0 and 1 in the switch statements), creating loop bitmaps that intersect but aren't subsets.

2. **Complex Control Flow**: Multiple `switch` statements with fall-through cases create shared basic blocks accessible from different loops. `goto` statements create early exits to labels outside immediate parent loops.

3. **Loop Transformations**: Explicit `#pragma GCC unroll` directives and manual unrolling patterns encourage the compiler to create multiple loop versions. The loop with computation → conditional access → computation pattern is designed to trigger loop distribution.

4. **Non-Constant Bounds**: Loop bounds use `volatile` variables and modulo operations to prevent constant propagation. Early exits via `goto` and `break` create additional exit blocks.

5. **Prevention of Optimization**: Functions marked with `__attribute__((noinline, noipa, optimize("O3")))` prevent inlining and inter-procedural analysis. Volatile arrays and global variables prevent dead code elimination.

6. **Multiple Loop Depths**: The recursive `generate_loops` function creates loops at depths 2, 3, and 4 when called from `main`. Multiple iterations with different parameters populate the compiler's loop tree with many candidates for parent-child analysis.

## Compilation Recommendations:
