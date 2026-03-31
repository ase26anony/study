Looking at this code, I can see the pattern you're describing. The variable `val` is defined by a **Phi node** at the start of each loop iteration because:

1. **SSA Form**: In Static Single Assignment form, each variable is assigned exactly once
2. **Phi Node**: At the start of the loop body, `val` needs a Phi node to merge values from:
   - The previous iteration's `val` (coming from the loop back edge)
   - The initial value `0` (first iteration)

The key insight is that `val` is **redefined in every iteration** based on `some_condition(i)`, but from the compiler's perspective in SSA form, this creates a Phi node at the start of the loop body.

Here's how the SSA transformation would look:

```llvm
// Initial value
val_initial = 0

for (i = 0; i < n; ++i) {
  // Phi node merges values from:
  // - val_prev (from previous iteration, via back edge)
  // - val_initial (first iteration)
  val_phi = Φ(val_initial, val_prev)
  
  if (some_condition(i)) {
    val_true = 1
  } else {
    val_false = 0
  }
  
  // Another Phi to merge the two branches
  val_after_if = Φ(val_true, val_false)
  
  // This becomes val_prev for next iteration
  val_prev = val_after_if
  
  if (val_after_if == 1) {
    // do work
  }
}
```

The condition `val == 1` is indeed:
- **LHS**: An SSA name (`val_after_if`) that comes from a Phi node
- **RHS**: A constant (`1`)

This is a classic pattern where value range analysis or other optimizations could potentially:
1. Propagate the constant value through the Phi node
2. Simplify the condition if possible
3. Potentially unroll or optimize the loop based on the predictable pattern

Is this the kind of analysis you're looking to perform on this pattern?
