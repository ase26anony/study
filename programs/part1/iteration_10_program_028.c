## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `switch` statements with shared cases (case 2 in both inner loops) create basic blocks that belong to multiple loops but not all blocks are shared.

2. **Partial Subset Relationships**: The second inner loop shares the handler block (case 2) with the first inner loop but has different other blocks (case 3), making neither loop's block set a complete subset of the other.

3. **Early Exits with `goto`**: The `goto early_exit` and `goto exit_loop` create control flow that exits loops to labels at different nesting levels, affecting loop membership of basic blocks.

4. **Recursive Loop Generation**: The `generate_loops` function creates varying nesting depths (2-4 levels) when called from `main`.

5. **Loop Distribution Candidates**: The mixed computation pattern (compute → conditional access → compute) encourages the compiler's loop distribution pass.

6. **Manual and Pragmatic Unrolling**: Both explicit unrolling and `#pragma GCC unroll` create multiple basic blocks within loops.

7. **Volatile Variables for Data-Dependent Bounds**: Loop bounds use `rand()` and volatile variables to prevent constant propagation.

## Compilation Recommendations:
