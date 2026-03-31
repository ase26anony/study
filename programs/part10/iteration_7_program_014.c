## Key Design Elements:

1. **GIMPLE_COND with Constant RHS**: The program contains multiple `if` statements comparing variables against constants 0 and 1 (`chain_var == 0`, `chain_var > 1`, `chain_var < 1`, `chain_var != 1`).

2. **SSA_NAME LHS**: All comparisons use simple variables (`chain_var`, `chain3`) that are SSA names, not complex expressions.

3. **Phi-Node Dependency Chain**: 
   - `phi_var` is created from a conditional assignment (`(i % 10 == 0) ? a : b`)
   - This creates a phi node at the loop header
   - A chain of simple assignments (`tmp1 = phi_var; tmp2 = tmp1; chain_var = tmp2`) creates the exact pattern the uncovered code looks for

4. **Auto-Profile Annotation**:
   - Hot functions marked with `__attribute__((hot))`
   - High iteration counts (100000, 50000) suggest hot loops
   - Cold function marked with `__attribute__((cold))` for contrast

5. **Multiple Conditional Patterns**: Four different comparison patterns against constants 0 and 1.

6. **Prevention of Early Optimization**:
   - `volatile` seed prevents compile-time computation
   - Trivial arithmetic (`base + i`, `base - i`) creates necessary data flow
   - Global volatile counters ensure side effects
   - Function arguments prevent constant propagation

## Compilation and Analysis:

To analyze the GIMPLE output and verify the patterns:
