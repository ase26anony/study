Looking at this code, the key insight is that `cond` is **modified inside the loop** (`cond = i`), and it's also used in the loop-invariant test `cond > 0` at the start of each iteration.

However, `cond` is **not loop-invariant** because it changes inside the loop body.  
Therefore, the compiler **cannot** hoist the `if (cond > 0)` check out of the loop.

Let's trace the first few iterations to see why:

1. **Iteration i=0**:  
   - `cond` starts at `some_value()` (unknown at compile time).  
   - If `cond > 0`, then `arr[0] = 0` and `cond = 0`.  
   - Now `cond` becomes 0.

2. **Iteration i=1**:  
   - `cond` is now 0, so `cond > 0` is false.  
   - The `if` block is skipped, `cond` stays 0.

3. **Iteration i=2, 3, ...**:  
   - `cond` remains 0, so `cond > 0` is always false for the rest of the loop.

So the loop will execute the `if` block **at most once** — only in the first iteration if the initial `cond > 0`. After that, `cond` is set to `i` (which is 0 in that first iteration), making it 0, and the condition fails for all later iterations.

Because `cond` can change inside the loop based on `i`, the compiler cannot assume the condition stays the same across iterations, so **hoisting is not safe**.

**Answer**: No, the compiler cannot hoist the `if (cond > 0)` out of the loop because `cond` is modified inside the loop body.
