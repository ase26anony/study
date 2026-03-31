## Key Design Elements:

1. **Overlapping Loop Blocks**: The `generate_loops` function creates two inner loops that share the `shared_handler` block through different control paths, creating partially overlapping block bitmaps.

2. **Complex Control Flow**: Multiple `switch` statements with fall-through cases and `goto` labels create shared basic blocks between loops, forcing `bitmap_intersect_compl_p` evaluations.

3. **Early Exits**: `goto exit_all_loops` and `goto outer_loop_continue` create exit blocks that belong to loops but transfer control outside.

4. **Loop Distribution Patterns**: The three-phase computation in the outer loop encourages the compiler's loop distribution pass to split it.

5. **Recursive Nesting**: The recursive call inside loops creates varying nesting depths, populating the loop tree with many candidates.

6. **Volatile Arrays**: Prevent optimization of memory operations and keep loop structures opaque.

7. **Manual Unrolling**: Creates additional basic blocks within loops.

## Compilation Recommendations:
