## Key Design Elements:

1. **PHI-Derived Conditional Branches**: Each pattern function creates situations where a conditional branch depends on a value defined by a PHI node at a control-flow merge point.

2. **SSA Copy Chains**: Variables like `temp1`, `temp2`, `temp3` create chains of simple assignments that the analysis must trace back through.

3. **Annotated Basic Blocks**: Nested loops and conditionals ensure rich control flow that should trigger basic block annotation.

4. **Constant RHS (0 or 1)**: All conditionals explicitly compare against `0` or `1` (e.g., `== 0`, `!= 1`, `== 1`).

5. **Multiple Variants**: Six different pattern functions exercise slightly different control flow structures.

6. **Main Function Pattern**: The `main()` function itself includes a while loop with a flag-based exit condition that follows the target pattern.

## Compilation Recommendations:
