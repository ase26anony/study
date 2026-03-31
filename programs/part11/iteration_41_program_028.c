Looking at this code, I can see you're describing a pattern where:

1. `val` is defined by a Phi node at the start of each loop iteration
2. The condition `val == 1` uses an SSA variable from a Phi node compared to a constant

This is indeed a classic pattern that compilers can optimize. The key insight is that `val` is only ever 0 or 1, and the condition `val == 1` is equivalent to just checking if `val` is truthy (non-zero).

In GCC/LLVM, this pattern might be optimized by:
- Recognizing that `val` is a boolean-like value (0 or 1)
- Simplifying `val == 1` to just `val != 0` or even just `val` in boolean context
- Potentially eliminating the Phi node entirely if the value is only used in this comparison

The optimization would work because:
- When `val = 1`, then `val == 1` is true
- When `val = 0`, then `val == 1` is false
- So `val == 1` is equivalent to the boolean value of `val` itself

Is this the kind of pattern/optimization you're asking about? Or are you looking for something more specific about how compilers handle Phi nodes in SSA form?
