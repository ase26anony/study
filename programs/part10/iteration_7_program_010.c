## Key Design Elements:

1. **Phi-Node Creation**: The conditional assignment `phi_var = (i % 10 == 0) ? a : b` creates a phi-node in GIMPLE IR, which the uncovered code traces back to.

2. **SSA Assignment Chain**: The sequence `tmp1 = phi_var; tmp2 = tmp1; chain_var = tmp2` creates the exact `GIMPLE_ASSIGN` chain with `gimple_assign_single_p` that the uncovered code looks for.

3. **Constant RHS Comparisons**: All conditionals compare against constants 0 or 1 (`== 0`, `> 1`, `< 1`, `!= 1`, `>= 1`, `<= 1`).

4. **Hot/Cold Attributes**: The `hot` attribute on `hot_loop_function` and `cold` on `cold_helper` help the auto-profile pass identify which basic blocks to annotate.

5. **Volatile Variables**: Using `volatile` for global counters and seed prevents aggressive optimization that might eliminate the desired control flow.

6. **Multiple Patterns**: The code includes all three required patterns (equality with 0, greater-than with 1, less-than with 1) plus additional variations.

7. **High Iteration Count**: The 100,000 iteration loop ensures the code is marked as "hot" during profiling.

## Compilation and Analysis:

To see the GIMPLE representation and verify the patterns:
