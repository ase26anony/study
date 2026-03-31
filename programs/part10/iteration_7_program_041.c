## Key Design Elements:

1. **GIMPLE_COND with Constant RHS**: The program creates multiple `if` statements comparing against constants 0 and 1:
   - `chain_var == 0` (equality with 0)
   - `chain_var > 1` (greater than 1)
   - `chain_var < 1` (less than 1)
   - `chain_var == 1` (equality with 1)

2. **SSA_NAME LHS**: All comparisons use simple SSA variables (`chain_var`) as the left-hand side, not complex expressions.

3. **Phi-Node Dependency Chain**: 
   - `phi_candidate` is assigned from either `ssa_var1` or `ssa_var2` based on a condition
   - This creates a phi-node at the loop header in GIMPLE representation
   - The chain `phi_candidate → chain1 → chain2 → chain_var` creates the required assignment chain

4. **Auto-Profile Annotation**:
   - `hot_function` marked with `__attribute__((hot))`
   - High iteration count (100,000) suggests hot loop
   - `cold_function` marked with `__attribute__((cold))` for contrast

5. **Multiple Conditional Patterns**: Four distinct comparison patterns against 0/1 constants.

6. **Anti-Optimization Measures**:
   - `volatile` global counters prevent dead code elimination
   - `volatile` seed prevents compile-time computation
   - Trivial arithmetic (`& 0xFF`) maintains data flow without simplification
   - Multiple function calls establish call graph for profiling

## Compilation and Analysis:

To analyze the GIMPLE output and verify the patterns:
