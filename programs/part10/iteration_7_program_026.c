## Key Design Elements:

1. **Phi-Node Creation**: The conditional assignment `phi_var = (i % 10 == 0) ? a : b` creates a phi-node at the GIMPLE level where `phi_var` gets either `a` or `b` based on the condition.

2. **SSA Assignment Chain**: Variables `tmp1`, `tmp2`, and `chain_var` form a chain of single assignments where each RHS is an SSA_NAME, matching the pattern:
