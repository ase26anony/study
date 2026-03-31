Looking at this code, I can see the pattern you're describing. The variable `val` is defined by a Phi node at the start of each loop iteration because:

1. **Phi node creation**: At the start of each iteration (after the loop header), `val` needs a value that depends on which path was taken in the previous iteration:
   - If `some_condition(i)` was true, `val` gets value 1
   - If `some_condition(i)` was false, `val` gets value 0

2. **SSA form**: In SSA (Static Single Assignment), `val` would be represented as different versions (`val_1`, `val_2`, etc.) at different points. The Phi node merges these versions.

3. **Pattern matching**: The condition `if (val == 1)` matches your target pattern because:
   - LHS (`val`) is an SSA_NAME that comes from a Phi node
   - RHS is a constant (1)
   - This is a comparison between a Phi result and a constant

In compiler terms, this creates a control-flow dependent value that could potentially be optimized. For example, if the compiler can prove something about `some_condition(i)`, it might optimize away the Phi node or the comparison.

The key insight is that `val`'s value at the point of the `if (val == 1)` check is determined by which branch was taken earlier in the same iteration, making it a classic Phi node scenario in SSA form.
