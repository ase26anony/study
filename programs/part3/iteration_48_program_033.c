## Key Observations:

1. **Phi Node Creation**: The compiler creates a phi node (`val_3`) that merges the values from both branches of the `if-else` statement.

2. **Constant Propagation Opportunity**: Since one branch always sets `val` to 0 and the other to 1, the phi node's value range is known to be `{0, 1}`.

3. **Condition Simplification**: The condition `val_3 == 0` can be analyzed as:
   - If `val_3` comes from Path A: `1 == 0` → `false`
   - If `val_3` comes from Path B: `0 == 0` → `true`
   
   So the condition is equivalent to checking whether we took Path B.

## Optimization Potential:

A smart compiler could potentially:
- **Hoist the condition**: Since the condition depends only on `some_condition(i)`, it could transform the code to:
