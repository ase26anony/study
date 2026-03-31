## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The shared block `arr1[50] = arr2[50] * 2` appears in both inner loops' switch statements, creating intersecting but not fully contained bitmaps.

2. **Complex Control Flow**: Multiple `switch` statements with `goto` early exits create complex CFGs that force loop hierarchy reconstruction.

3. **Loop Distribution Candidates**: The mixed computation patterns (arithmetic, bitwise ops, conditional stores) encourage the compiler's loop distribution pass.

4. **Volatile Variables**: Used for loop bounds and array accesses to prevent invariant code motion and keep loops opaque.

5. **Recursive Depth Variation**: The `generate_loops` function creates different nesting depths (2-4 levels) across iterations.

6. **Multiple Loop Patterns**: Two different complex loop functions (`generate_loops` and `nested_switch_loops`) increase the diversity of loop structures.

## Compilation Recommendations:
