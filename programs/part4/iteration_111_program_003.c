Looking at this code, I can see there's a potential issue with the loop's behavior. Let me analyze it:

**The Problem:**
The variable `cond` is being modified inside the loop (`cond = i`), but it's also being tested in the condition `if (cond > 0)`. This creates a situation where the loop's behavior changes dynamically based on its own execution.

**How it works:**
1. Initially, `cond` has some initial value (`some_value()`)
2. On each iteration:
   - If `cond > 0` is true, it executes the then block
   - Inside the then block, it sets `cond = i` (current loop index)
   - This means `cond` will keep increasing as the loop progresses

**Potential outcomes:**
- If initial `cond > 0`: The loop will execute the then block on the first iteration, setting `cond = 0`. Then `cond > 0` becomes false, so all subsequent iterations (i=1 to 99) will skip the then block.
- If initial `cond ≤ 0`: The loop will skip the then block entirely for all iterations.

**So essentially:**
- The then block will execute at most **once** (on the first iteration if initial `cond > 0`)
- After that first execution, `cond` becomes 0, making the condition false for all remaining iterations

**The array `arr` will be:**
- Mostly uninitialized (contains garbage values)
- Only `arr[0]` might be set to 0 if initial `cond > 0`

This is likely a bug unless the intended behavior is to execute the then block only once. If the goal was to fill the array with values based on some condition, the modification `cond = i` inside the loop is probably incorrect.
