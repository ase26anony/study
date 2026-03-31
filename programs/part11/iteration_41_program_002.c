## Key Observations:

1. **Phi Node Creation**: The `val` variable gets a new SSA value in each iteration:
   - `%val.phi` at loop entry (initialized to 0)
   - `%val.next` after the if-else merge block

2. **Pattern Recognition**: The condition `if (val == 1)` is exactly what you described:
   - **LHS**: `%val.next` (SSA_NAME from a Phi node)
   - **RHS**: Constant `1`
   - **Operation**: Equality comparison

3. **Optimization Opportunity**: This pattern is often optimized by:
   - Propagating the constant value through the Phi node
   - Simplifying the condition to directly use `some_condition(i)` result
   - Eliminating the intermediate `val` variable entirely

## Optimized Version:

A smart compiler might transform this to:
