## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that both contain the same switch statement with a shared handler block (case 1). This creates loops whose basic block sets intersect but neither is a subset of the other.

2. **Complex Control Flow**: Multiple `goto` statements create edges between loops, and early exits (`break` from inner loops) create basic blocks that belong to a loop but transfer control outside it.

3. **Loop Distribution Pattern**: The computation-then-conditional-access pattern in the second inner loop encourages the compiler's loop distribution pass to potentially split it.

4. **Recursive Depth Variation**: The recursive calls with depths 2, 3, and 4 create loops at different nesting levels, populating the loop tree with many candidates for parent-child analysis.

5. **Cross-Loop Shared Blocks**: The `cross_loop_pattern` function has two separate loops that can both jump to the same basic block (`shared_block2`), creating the partial overlap needed for `bitmap_intersect_compl_p`.

6. **Volatile Arrays and Global**: Using volatile arrays prevents optimization of memory operations, and the changing `global_seed` prevents merging of loops across iterations.

## Compilation Recommendations:
