## Key Points:

1. **PHI Node**: The `%val` in the merge block is a PHI node that selects between `%val.then` (1) and `%val.else` (2) based on which predecessor block was executed.

2. **SSA Property**: Each variable is assigned exactly once, so we need different names (`%val.then`, `%val.else`) in different branches, then merge them with a PHI node.

3. **Value Propagation**: A good optimizer would likely:
   - Recognize that `%val == 1` is equivalent to checking if we came from the `then` block
   - Potentially eliminate the PHI node through constant propagation
   - Simplify the control flow

## Optimized Version:

An optimizer might transform this to:
