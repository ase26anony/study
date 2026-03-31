## Key Design Elements:

1. **Partially Overlapping Loop Blocks**: The `generate_nested_loops` function creates two inner loops that share the `shared_handler` block via `goto`, but have different surrounding code, creating bitmaps that intersect but aren't subsets.

2. **Complex Control Flow**: Multiple `switch` statements and `if-else` chains inside loops create many basic blocks with conditional entry points from different loops.

3. **Loop Transformations**: Manual unrolling with `#pragma GCC unroll`, loop distribution candidates (separate computation phases), and early exits via `goto`.

4. **Non-Constant Bounds**: Loop bounds use `volatile` variables and modulo operations to prevent constant propagation.

5. **Multiple Loop Hierarchies**: The recursive `generate_nested_loops` creates loops at depths 2, 3, and 4, while `overlapping_loop_clusters` creates another pattern of overlapping loops.

6. **Anti-Optimization**: `volatile` arrays, `noinline/noipa` attributes, and global volatile variables prevent premature optimization and loop merging.

## Compilation Recommendations:
