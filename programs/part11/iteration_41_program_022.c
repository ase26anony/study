Looking at this code, I can see the pattern you're describing. In SSA (Static Single Assignment) form, the variable `val` would indeed require a Phi node at the beginning of each loop iteration because:

1. **SSA requirement**: Each variable can only be assigned once
2. **Loop creates multiple assignments**: `val` is assigned in both branches of the if-statement
3. **Phi node needed**: At the start of each iteration, `val` needs to merge values from:
   - The previous iteration's `val` (for the loop back edge)
   - The initial value `val = 0` (for the first iteration)

The SSA transformation would look something like:

```llvm
val_initial = 0
for i = 0; i < n; ++i:
    val_phi = Φ(val_initial, val_prev)  // Phi node at loop header
    if (some_condition(i)):
        val_true = 1
    else:
        val_false = 0
    val_merge = Φ(val_true, val_false)  // Phi node after if-else
    val_prev = val_merge  // For next iteration
    
    if (val_merge == 1):
        // do work
```

**Key insight**: The comparison `val == 1` is comparing an SSA value that comes from a Phi node (which merges values from different control flow paths) against a constant. This is exactly the pattern that can be optimized by converting the Phi node into a select instruction or by using predicated execution.

This pattern is common in compiler optimizations like:
- **Phi-to-select conversion**: Turning control-dependent Phi nodes into data-dependent select operations
- **Predication**: Using conditional moves instead of branches
- **If-conversion**: Converting control flow into straight-line code with predicated operations
