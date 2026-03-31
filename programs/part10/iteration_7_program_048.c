## Key Design Elements:

1. **Phi-Node Creation**: The `if (i % 10 == 0)` construct creates a phi-node where `phi_var` gets either `a` or `b`.

2. **SSA Assignment Chain**: The sequence `tmp1 = phi_var; tmp2 = tmp1; chain_var = tmp2;` creates exactly the `GIMPLE_ASSIGN` chain with `gimple_assign_single_p` that the uncovered code traces through.

3. **Constant RHS Comparisons**: All conditionals compare against constants 0 or 1 (`== 0`, `> 1`, `< 1`, `!= 1`).

4. **Hot/Cold Annotations**: Functions marked with `__attribute__((hot))` and `__attribute__((cold))` help the auto-profile pass identify which basic blocks to annotate.

5. **Volatile Variables**: Using `volatile` prevents compile-time optimization of the initialization values.

6. **High Iteration Counts**: The loops iterate 100,000 and 50,000 times to ensure they're marked as "hot" during profiling.

## Compilation Commands:
