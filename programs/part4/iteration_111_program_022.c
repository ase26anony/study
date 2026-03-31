Looking at this code, the key insight is that `cond` is **modified inside the loop** (`cond = i`), and it's also used in the loop-invariant test `if (cond > 0)`.

However, the compiler cannot hoist the `if (cond > 0)` check out of the loop because:

1. **`cond` is modified inside the loop** (in the then-block: `cond = i`)
2. This modification happens **only when the condition is true**
3. The next iteration's condition check depends on the updated value of `cond`

So the condition is **not loop-invariant** - it can change each iteration based on whether the then-block executed in the previous iteration.

**Example trace:**
- If `cond = 5` initially: iteration 0 checks `5 > 0` (true), sets `cond = 0`
- Iteration 1 checks `0 > 0` (false)
- Iteration 2 checks `0 > 0` (false), etc.

The condition becomes false after the first iteration where `i = 0`.

**Therefore:**
- ❌ **Cannot hoist** `if (cond > 0)` out of the loop
- The check must remain inside each iteration because `cond` is a **loop-carried dependency**

The compiler might still optimize by:
- Keeping the condition check inside the loop
- Possibly unrolling or using other optimizations, but not hoisting this particular condition check
