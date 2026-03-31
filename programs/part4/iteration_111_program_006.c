Looking at this code, I can see there's a potential issue with the loop optimization. Let me analyze it:

**Key observations:**
1. `cond` is used in the loop condition (`if (cond > 0)`)
2. `cond` is modified inside the loop (`cond = i`)
3. This creates a **loop-carried dependency** - the value of `cond` in iteration `i+1` depends on what happened in iteration `i`

**Why this prevents certain optimizations:**

1. **Cannot hoist the condition check outside the loop**: A compiler might want to move `cond > 0` outside the loop if `cond` doesn't change, but here it does change.

2. **Cannot unroll or parallelize easily**: Since each iteration's condition depends on the previous iteration's execution, the loop iterations aren't independent.

3. **Cannot vectorize**: Vectorization requires independent iterations, but here iteration `i+1` needs to know if iteration `i` executed the `then` block to get the correct `cond` value.

**What the compiler sees:**
- `cond` is a variable that's both read and written in the loop
- The read (`cond > 0`) depends on the write (`cond = i`)
- This creates a data dependency that forces sequential execution

**If `cond` were not modified inside the loop**, the compiler could:
- Hoist the condition check: `if (cond > 0) { for (...) { arr[i] = i; } }`
- Potentially unroll or vectorize the loop
- Generate more efficient code

**Current behavior**: The loop must execute sequentially because:
- Iteration 1: checks original `cond` value
- If `cond > 0`, sets `cond = 0` (since `i = 0`)
- Iteration 2: checks `cond` (now 0), so `cond > 0` is false
- All subsequent iterations: `cond` remains 0, so condition is false

So the loop will only execute the `then` block at most once (on the first iteration if the original `cond > 0`).
