## Pattern Recognition

The key pattern you're highlighting is:
1. `val` is defined by a Phi-like construct (different assignments in different branches)
2. Immediately after the merge point, there's a comparison `val == 1`
3. This creates a predictable branch pattern

## Optimization Opportunity

This pattern is interesting because:
- The comparison `val == 1` is redundant in some sense - we already know which branch we came from
- In the `then` branch, `val` is always 1, so `val == 1` is always true
- In the `else` branch, `val` is always 0, so `val == 1` is always false

A smart compiler could potentially:
1. **Hoist the condition**: Move the `if (val == 1)` check into the branches where `val` is assigned
2. **Eliminate the Phi**: Instead of storing to `val` and then comparing, directly branch based on `some_condition(i)`
3. **Simplify control flow**: Merge the "do work" into the `then` branch directly

The optimized version would look like:
