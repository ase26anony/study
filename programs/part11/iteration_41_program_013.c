Looking at this code, I can see the pattern you're describing:

1. **Phi Node Creation**: Inside the loop, `val` gets assigned different values (1 or 0) based on `some_condition(i)`. This creates a situation where at the start of each iteration, `val` needs a phi node to merge the value from the previous iteration with the initial value.

2. **Pattern Matching**: The condition `if (val == 1)` exactly matches the pattern you mentioned:
   - LHS (`val`) is an SSA_NAME that comes from a phi node
   - RHS is a constant (1)

3. **SSA Form Transformation**: In SSA form, this would look something like:
   ```llvm
   val_0 = 0  // initial value
   for (i = 0; i < n; i++) {
     val_phi = φ(val_0, val_2)  // phi node at start of loop body
     
     if (some_condition(i)) {
       val_1 = 1
     } else {
       val_1 = 0
     }
     val_2 = val_1  // value for next iteration
     
     if (val_phi == 1) {  // Using phi result in comparison
       // do work
     }
   }
   ```

The key insight is that the comparison `val == 1` uses the value from the phi node (`val_phi`), which merges the two possible incoming values (1 from the true branch, 0 from the false branch) from the previous iteration's control flow.

This is a classic pattern that compiler optimizations might look for, such as:
- **Condition simplification**: If the compiler can prove something about `some_condition(i)`, it might optimize the comparison
- **Loop-invariant code motion**: If `some_condition(i)` doesn't depend on loop-invariant values, the comparison result might be hoisted
- **Dead code elimination**: If the comparison always evaluates the same way, one branch might be removed
