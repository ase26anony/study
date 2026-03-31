## Key Design Elements:

1. **PHI-Derived Conditionals**: Each pattern function creates variables that become PHI nodes at control flow merge points (loop headers, if-else merges).

2. **SSA Copy Chains**: Variables like `temp1`, `temp2`, `temp3` create the copy chains that the uncovered code traces through.

3. **Constant RHS Comparisons**: All conditionals compare against `0` or `1` explicitly (`== 0`, `== 1`, `!= 1`) or implicitly (`if (propagated)`).

4. **Annotated Basic Blocks**: Nested loops, multiple branches, and complex control flow ensure basic blocks get marked for profiling.

5. **Multiple Variants**: Five different pattern functions with slight variations increase coverage probability.

6. **Executable Main**: The `main()` function exercises all patterns with varying inputs and includes its own PHI pattern.

## Compilation Recommendations:
