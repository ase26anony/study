## The Optimization Pattern
The key insight is that `val` is defined by a Phi node with two constant incoming values: `1` and `0`. When we check `val == 1`, we're essentially checking which incoming edge was taken.

This pattern can be optimized because:
1. The Phi node has constant incoming values
2. The comparison is with one of those constants
3. The result of the comparison is equivalent to checking which incoming edge was taken

## Possible Optimizations

### 1. **Direct Branch Optimization**
Instead of computing the Phi value and then comparing it, we could directly use the condition that determined the Phi value:
