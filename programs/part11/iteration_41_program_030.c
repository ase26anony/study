## Key Observations:

1. **Phi Node Creation**: The variable `val` needs a phi node because:
   - It's defined in multiple places within the loop (both branches of the if)
   - The value from the previous iteration flows into the current iteration

2. **Pattern Recognition**: The condition `val == 1` is exactly the pattern you described:
   - LHS (`val`) is an SSA name that comes from a phi node
   - RHS is a constant (`1`)
   - This is a classic case for phi-based optimization

3. **Optimization Opportunity**: A compiler could potentially:
   - Propagate the constant through the phi node
   - Simplify the condition based on which branch was taken
   - Possibly eliminate the phi node if the condition is predictable

## Alternative SSA Representation (more optimized):

Actually, looking more carefully, a better SSA representation might be:
