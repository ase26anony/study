## Key Design Elements for Triggering the Target Code:

1. **Overlapping Loop Bitmaps**: The two inner loops in `create_complex_loop_hierarchy` share the `shared_label` block and the `case 1:` handler in the switch statement, creating bitmaps that intersect but aren't subsets.

2. **Complex Control Flow**: Multiple `switch` statements with `fallthrough` attributes and `goto` jumps to shared labels create basic blocks that belong to multiple loops' bitmaps.

3. **Loop Transformations**: Manual unrolling, `#pragma GCC unroll`, and loops structured for potential distribution (computation → conditional access → more computation).

4. **Non-Constant Bounds**: Volatile variables and arithmetic expressions for loop bounds prevent constant propagation and folding.

5. **Multiple Loop Candidates**: Recursive function creates loops at depths 2, 3, and 4, populating the compiler's loop tree with many candidates for relationship analysis.

6. **Early Exits**: `goto outer_exit` and `goto skip_rest` create control flow that exits loops to outside their immediate parents.

## Compilation Recommendations:
