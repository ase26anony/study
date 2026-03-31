## Key Points:

1. **Phi Node Creation**: At the merge point after the if-else, the compiler creates a phi node that selects between:
   - `val_A = 1` (from the true branch)
   - `val_B = 0` (from the false branch)

2. **Constant Propagation Opportunity**: 
   - The compiler can see that `val_phi` can only be either 0 or 1
   - The condition `val_phi == 0` will be true only when `some_condition(i)` is false

3. **Potential Optimization**: A smart compiler might transform this to:
