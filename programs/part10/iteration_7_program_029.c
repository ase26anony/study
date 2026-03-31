## Key Design Elements:

1. **Phi-Node Creation**: The `phi_var` is conditionally assigned from either `a` or `b` inside the loop, creating a phi-node at the loop header in GIMPLE representation.

2. **SSA Assignment Chain**: The `tmp1 = phi_var; tmp2 = tmp1; chain_var = tmp2;` sequence creates exactly the chain of `GIMPLE_ASSIGN` statements with single SSA-to-SSA assignments that the uncovered code traces through.

3. **Constant Comparisons**: Multiple `if` statements compare `chain_var` against constants 0 and 1 using different relational operators (`==`, `>`, `<`, `!=`).

4. **Hot/Cold Annotations**: `__attribute__((hot))` on `hot_loop_function` and `__attribute__((cold))` on `cold_helper` help the auto-profile pass identify which basic blocks to annotate.

5. **Volatile Input**: Using `volatile int seed` prevents the compiler from computing everything at compile-time, preserving the SSA chains and phi-nodes.

6. **High Iteration Count**: The loop runs 100,000 times, making it likely to be profiled as a hot loop.

## Compilation and Verification:

To verify the GIMPLE output contains the desired patterns:
