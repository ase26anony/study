Looking at this code, I can see the pattern you're describing:

1. **Phi Node Creation**: Inside the loop, `val` is assigned different values (1 or 0) based on `some_condition(i)`. This creates a situation where at the start of each iteration, `val` needs a phi node to merge the value from the previous iteration with the newly assigned value.

2. **Pattern Matching**: The condition `if (val == 1)` has exactly the pattern you mentioned:
   - LHS (`val`) is an SSA_NAME that comes from a phi node
   - RHS is a constant (1)

3. **Optimization Opportunity**: This is a classic case where the phi node and comparison could potentially be optimized. The compiler could recognize that:
   - `val` is only ever 0 or 1
   - The comparison `val == 1` is equivalent to checking if `val` is truthy/non-zero
   - Since `val` is a boolean-like value (0 or 1), the comparison might be redundant

4. **Possible Optimizations**:
   - **Phi node elimination**: If the only use of `val` is in the comparison `val == 1`, the compiler might eliminate the phi node entirely and use `some_condition(i)` directly
   - **Comparison simplification**: The comparison could be simplified since `val` is already a boolean value
   - **Dead code elimination**: If `val` has no other uses, the assignments to `val` might be eliminated

The key insight is that this pattern allows the compiler to trace the flow from the condition (`some_condition(i)`) through the phi node to the comparison, potentially allowing optimizations that bypass the intermediate phi node entirely.
