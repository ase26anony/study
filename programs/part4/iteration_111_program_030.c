Looking at this code, I can see that the condition `cond > 0` is evaluated in every loop iteration, but `cond` is modified inside the loop body. This creates a **loop-variant condition** - the condition's value can change during loop execution.

Here's what happens:

1. **Initial check**: `cond > 0` is evaluated based on `some_value()`
2. **If true**: 
   - `arr[i] = i` is executed
   - `cond = i` updates the condition variable
3. **Next iteration**: The condition is re-evaluated with the new value of `cond`

This means the loop's behavior depends on:
- The initial value of `cond`
- How `cond` changes during execution

**Key observations:**
- If `cond` starts as `≤ 0`, the loop body never executes
- If `cond` starts as `> 0`, the loop will execute at least once
- Once `cond` becomes `≥ 100`, the condition will be false (since `i < 100`)
- The loop could potentially stop early if `cond` becomes `≤ 0` during execution

**Example scenarios:**
- If `cond = 1`: Loop executes for all `i` from 0 to 99
- If `cond = 50`: Loop executes for `i` from 50 to 99
- If `cond = -1`: Loop never executes

This pattern is sometimes used for **conditional loop execution** where you want to:
1. Start executing only when a certain condition is met
2. Potentially stop early based on computed values
3. Track progress through the loop variable

However, this code can be confusing because `cond` serves two purposes:
- Initial condition check
- Tracking the last index processed
