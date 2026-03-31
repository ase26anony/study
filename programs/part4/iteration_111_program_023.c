Looking at this code, I can see the issue: **the condition `cond > 0` is not loop-invariant** because `cond` is modified inside the loop body.

Let me break it down:

## Problem Analysis

1. **Initial condition**: `cond = some_value()`
2. **Loop condition**: `i < 100`
3. **If condition**: `cond > 0`
4. **Inside the if-block**: `cond = i` modifies the condition variable

## Why This Matters

For compiler optimizations like **loop-invariant code motion**, the compiler needs to determine if an expression's value remains constant throughout all loop iterations. Here:

- `cond > 0` depends on `cond`
- `cond` is modified inside the loop (`cond = i`)
- Therefore, `cond > 0` could change value in each iteration
- This makes it **not loop-invariant**

## Example Execution

If `some_value()` returns 5:
- Iteration 0: `cond = 5`, `cond > 0` is true, `arr[0] = 0`, `cond = 0`
- Iteration 1: `cond = 0`, `cond > 0` is false, skip if-block
- Iteration 2+: `cond` remains 0, `cond > 0` is always false

The loop behavior changes after the first iteration!

## Optimization Implications

A compiler **cannot** safely hoist `cond > 0` out of the loop because:
1. The value changes inside the loop
2. The condition affects control flow (determines whether the if-block executes)
3. Moving it could change program semantics

## Key Insight

The variable `cond` serves two purposes:
1. **Control variable**: Determines whether to execute the if-block
2. **Data variable**: Stores the current index `i` when the if-block executes

This dual role prevents the condition from being loop-invariant.
