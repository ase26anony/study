## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that share the `if (control % 3 == 0)` block and the `early_exit` label, but have different computations. This creates loops with intersecting but not fully contained block sets.

2. **Complex Control Flow**: Multiple `switch` statements with fall-through cases create shared basic blocks between different loops. The `nested_switch_loops` function has an outer and inner loop sharing the same switch case blocks.

3. **Loop Transformations**: Manual unrolling (4 iterations) and the `loop_fission_candidate` function with distinct computation phases encourage compiler loop distribution.

4. **Non-Constant Bounds & Early Exits**: Loop bounds use `volatile` variables and `rand()`. The `goto early_exit` creates exits to labels outside immediate parent loops.

5. **Prevention of Optimization**: All critical functions have `__attribute__((noinline, noipa, optimize("O3")))`. Arrays are `volatile` and passed as pointers.

6. **Multiple Loop Depths**: The recursive `generate_loops` is called with depths 2, 3, and 4, creating varied loop hierarchies.

## Compilation Recommendations:
