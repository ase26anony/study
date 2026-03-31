## Key Design Elements:

1. **Overlapping Loop Blocks**: The `shared_block` labels and `switch` statements create basic blocks that belong to multiple loops but aren't perfectly nested.

2. **Complex Control Flow**: Multiple `goto` statements create irreducible control flow, forcing the compiler to rebuild loop hierarchies.

3. **Partial Overlap Pattern**: The three loops in `overlapping_loop_cluster` intersect but none is a strict subset of another, triggering `bitmap_intersect_compl_p` checks.

4. **Recursive Loop Generation**: The `generate_loops` function creates varying nesting depths, populating the loop tree with many candidates for comparison.

5. **Volatile and Pointer Aliasing**: Prevents optimization from simplifying loop structures too early.

6. **Early Exits**: Multiple `break`, `goto`, and conditional exits create additional basic blocks that affect bitmap calculations.

## Compilation Recommendations:
