## Key Points:

1. **Control Flow Merge**: The `if-else` creates two paths that merge at the point where `val` is used
2. **Single Definition**: In SSA, `val` can only be defined once, so the compiler creates a phi function at the merge point
3. **Runtime Selection**: The phi function selects between `some_computation()` and `another_computation()` based on which path was taken

## In Your Code:
