## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that share the `shared_handler` basic block via `goto` statements, but each also has unique blocks. This creates the "intersect but not subset" condition needed for `bitmap_intersect_compl_p`.

2. **Complex Control Flow**: Multiple `switch` statements with fall-through cases, `goto` jumps to shared labels, and early `break`/`continue` statements create a web of basic blocks that can belong to multiple loops.

3. **Loop Transformations**: Manual 4x unrolling, `#pragma GCC unroll`, and mixed computation patterns (computation → conditional access → more computation) encourage loop distribution.

4. **Non-Constant Bounds**: Loop bounds use `volatile` variables, `rand()`, and modulo operations to prevent constant propagation.

5. **Multiple Loop Candidates**: The recursive `generate_loops` creates loops at depths 2-4, and `main` calls it multiple times with different patterns via `run_loop_variants`.

6. **Anti-Optimization**: `__attribute__((noinline, noipa))`, volatile arrays, pointer arithmetic with modulo, and global volatile `control` variable prevent premature optimization.

## Compilation Recommendations:
