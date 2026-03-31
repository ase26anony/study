Looking at this code, I can see it's demonstrating a pattern where SSA (Static Single Assignment) form would create a Phi (Φ) node for `val` at the beginning of each loop iteration.

Let me trace through the SSA transformation:

## Original Code Analysis:
1. `val` is initialized to `0` before the loop
2. Inside the loop, `val` gets assigned either `1` or `0` based on `some_condition(i)`
3. The `if (val == 1)` check uses the value from the Phi node

## SSA Transformation:
In SSA form, each variable can only be assigned once. So `val` becomes multiple versions:

```cpp
int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_phi;  // Phi node at start of block
    if (some_condition(i)) {
        int val_1 = 1;  // One incoming value to Phi
        val_phi = val_1;
    } else {
        int val_2 = 0;  // Other incoming value to Phi
        val_phi = val_2;
    }
    // val_phi is the result of the Phi node
    if (val_phi == 1) {  // Using Phi result
        // do work
    }
}
```

## Key Observations:
1. **Phi Node Location**: The Phi node conceptually exists at the beginning of the loop body block, merging values from:
   - The previous iteration's `val_phi` (or initial `val_0` for first iteration)
   - The `val_1` from the true branch
   - The `val_2` from the false branch

2. **Pattern Recognition**: The condition `val == 1` is interesting because:
   - `val` comes from a Phi node
   - The comparison is against a constant (`1`)
   - One of the Phi's incoming values is exactly that constant (`1`)
   - The other incoming value is a different constant (`0`)

3. **Optimization Opportunity**: A compiler could potentially optimize this by:
   - Recognizing that `val_phi == 1` is equivalent to `some_condition(i) == true`
   - Eliminating the Phi node entirely
   - Simplifying to: `if (some_condition(i)) { // do work }`

This is a classic example of how SSA form enables certain optimizations by making data flow explicit through Phi nodes and constant propagation.
