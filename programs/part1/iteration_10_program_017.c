## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `switch` statements with shared handler blocks (case 1) create basic blocks that belong to multiple loops' bitmaps but aren't fully contained in any single loop.

2. **Partial Loop Nesting**: The second inner loop shares the switch handler with the first inner loop but has different surrounding code, creating `intersect_compl_p` conditions.

3. **Complex Control Flow**: `goto early_exit`, `break`, and `continue` statements create additional edges in the CFG, affecting loop bitmap calculations.

4. **Loop Distribution Patterns**: The three-phase loop (computation → conditional access → computation) encourages compiler loop fission.

5. **Multiple Loop Hierarchies**: Recursive calls with different depths populate the compiler's loop tree with varied structures.

6. **Anti-Optimization Barriers**: `volatile` arrays, pointer arithmetic, and global variables prevent premature optimization and loop invariant code motion.

## Recommended Compilation:
