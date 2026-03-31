## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The two inner loops in `complex_loop_hierarchy` share the conditional block `if ((i + j) % 3 == 0)`, creating partially overlapping but not fully contained block sets.

2. **Complex Control Flow**: Multiple `switch` statements and `if-else` chains create shared basic blocks (like `arr3[1] = ...`) that are reachable from different loops.

3. **Loop Transformations**: Manual unrolling (4 iterations), `#pragma GCC unroll`, and mixed computation patterns encourage loop distribution.

4. **Non-Constant Bounds & Early Exits**: Loop bounds use `control % N` expressions, and `goto early_exit` creates early exits that affect loop bitmaps.

5. **Prevention of Optimization**: `volatile` arrays, `__attribute__((noinline, noipa))`, and arithmetic with volatile pointers prevent premature optimization.

6. **Multiple Loop Depths**: The recursive `recursive_loop_generator` creates loops at varying depths (2-4), populating the compiler's loop tree.

## Compilation Recommendations:
