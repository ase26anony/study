## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that share the same `switch` statement structure and `if` blocks, but with different unrolling patterns and bounds. This creates loops whose basic block bitmaps intersect but neither is a subset of the other.

2. **Complex Control Flow**: The `switch` statements with fall-through cases and shared handler blocks (`if (arr1[idx] > 100)`) create basic blocks that belong to multiple loops' bitmaps.

3. **Early Exits with `goto`**: The `goto early_exit` statements allow inner loops to exit to a label outside their immediate parent, creating control flow edges that affect loop bitmap calculation.

4. **Mixed Loop Transformations**: Manual unrolling, `#pragma GCC unroll`, and loop distribution candidates (the mixed computation/conditional/access loop) encourage the compiler to split and reorganize loops.

5. **Data-Dependent Bounds**: Loop bounds use `volatile` variables and modulo operations, preventing constant propagation and forcing dynamic analysis.

6. **Recursive Depth Variation**: Multiple calls with depths 2, 3, and 4 populate the compiler's loop tree with loops at different nesting levels.

7. **Volatile Arrays and Global**: Prevent optimization and ensure memory operations remain in loops.

## Compilation Recommendations:
