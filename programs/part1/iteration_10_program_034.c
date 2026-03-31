## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_complex_loops` function creates two inner loops that share the `arr2[idx] ^= arr1[idx]` computation block via the switch statement, but each loop also has unique blocks. This creates the partial overlap needed for `bitmap_intersect_compl_p`.

2. **Complex Control Flow**: Multiple `switch` statements, `goto` jumps (`early_exit`, `outer_break`), and `continue` statements create additional basic blocks that belong to loops but transfer control in non-nested ways.

3. **Loop Transformations**: Manual unrolling, `#pragma GCC unroll`, and loop fission patterns (separate computation, conditional, computation) encourage the compiler's distribution passes.

4. **Non-Constant Bounds**: Loop bounds use `volatile` variables and modulo operations, preventing constant propagation and forcing dynamic analysis.

5. **Multiple Loop Hierarchies**: The recursive generator creates loops at depths 2, 3, and 4, populating the compiler's loop tree with many candidates for parent-child analysis.

6. **Anti-Optimization**: `volatile` arrays, pointer arithmetic with potential aliasing, and `noinline`/`noipa` attributes prevent premature simplification.

## Compilation Recommendations:
