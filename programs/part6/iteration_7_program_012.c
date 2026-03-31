Looking at this code, I can see the issue you're pointing out: **modifying the condition variable `a` inside the "then" block**.

Here's what happens:

1. **Initial check**: `if (a > 0)` evaluates based on the original value of `a`
2. **If true**: 
   - `b = 10` gets executed
   - `a = 5` modifies the condition variable
3. **If false**: `b = 20` gets executed

The key insight is that **changing `a` inside the "then" block doesn't affect the control flow** - the `else` block won't suddenly execute just because `a` was modified. The condition was already evaluated once at the beginning.

This is a common code smell because:
- It can confuse readers who might think the condition could be re-evaluated
- It modifies a variable used in a condition, which can lead to maintenance issues
- The variable `a` has different meanings at different points in execution

**Better practice**: Use a different variable or restructure the logic to avoid modifying condition variables within conditional blocks.
