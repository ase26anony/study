## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The two inner loops in `generate_nested_loops` share the same `switch` statement structure, creating basic blocks that belong to both loops but aren't fully contained in either.

2. **Complex Control Flow**: Multiple `switch` statements, `if-else` chains, and `goto` statements create control flow that forces the compiler to rebuild loop hierarchies.

3. **Loop Transformations**: Manual unrolling, `#pragma GCC unroll`, and computation patterns that encourage loop distribution.

4. **Non-Constant Bounds**: `volatile` bounds and `rand()` calls prevent constant propagation and create data-dependent loop structures.

5. **Multiple Nesting Depths**: The recursive `generate_nested_loops` function creates loops at different depths, populating the loop tree with candidates for comparison.

6. **Early Exits**: `goto` statements that jump outside immediate parent loops affect loop bitmap calculations.

## Compilation Recommendations:
