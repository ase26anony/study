## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The two inner loops in `generate_loops()` share the `switch` statement's case blocks, creating loops whose basic block sets intersect but neither is a subset of the other.

2. **Complex Control Flow**: Multiple `goto` statements (`early_exit`, `shared_block`) create early exits and shared basic blocks between loops.

3. **Loop Transformations**: Manual unrolling, `#pragma GCC unroll`, and mixed operations encourage loop distribution/fission.

4. **Data-Dependent Bounds**: Loop bounds use `volatile` variables and modulo operations to prevent constant propagation.

5. **Recursive Nesting**: The `generate_loops()` function creates loops at different depths, populating the compiler's loop tree.

6. **Volatile Arrays**: All array accesses use `volatile` to prevent optimization and maintain loop structure.

## Recommended Compilation:
