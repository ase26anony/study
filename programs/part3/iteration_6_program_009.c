## Key Points:

1. **PHI Node**: The `%val` in the merge block is indeed a PHI node that selects between `%val.then` (1) and `%val.else` (2) based on which predecessor block was executed.

2. **Value Propagation**: A good compiler would optimize this by:
   - Recognizing that `%val` can only be 1 or 2
   - Seeing that `%val == 1` is equivalent to checking if we came from the `then` block
   - Potentially eliminating the PHI node and comparison through constant propagation

3. **Optimization**: This could be optimized to:
