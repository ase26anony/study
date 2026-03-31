## Key Points

- **Guaranteed initialization**: `val` is guaranteed to be initialized on all paths before being used
- **Single definition point**: From the perspective of the second `if` statement, `val` appears to have a single definition point (the phi node)
- **Data flow**: The second condition depends on data that flows through the control flow merge point

## Optimization Opportunities

Compilers can optimize this by:
1. **Value range propagation**: Tracking possible values of `val` from both computation functions
2. **Code motion**: Potentially moving computations if they don't have side effects
3. **Branch fusion**: In some cases, combining the two `if` statements

## Equivalent Straight-line Code

Conceptually, this is equivalent to:
