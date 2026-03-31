## Key Observations:

1. **Phi Node Creation**: The variable `val` gets different values in different branches (`then` and `else`), so a Phi node (`%val = phi i32 [1, %then], [0, %else]`) is created at the merge point.

2. **Pattern Recognition**: The condition `if (val == 1)` creates an `icmp eq` instruction comparing the Phi result (`%val`) with constant `1`.

3. **Optimization Opportunity**: This is a classic pattern that compilers can optimize. Since `val` can only be `0` or `1` from the Phi, the comparison `val == 1` is essentially checking which branch was taken.

## Possible Optimizations:

1. **Branch Folding**: The compiler could eliminate the Phi and comparison by directly using the condition:
