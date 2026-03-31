Looking at this code, I can see the issue you're pointing out: **the condition variable `a` is modified inside the "then" block**.

Here's what happens:

1. **Initial check**: `if (a > 0)` evaluates based on the original value of `a`
2. **If true**: 
   - `b = 10` is executed
   - `a = 5` changes the value of `a` (this is the problematic modification)
3. **If false**: `b = 20` is executed

The key problem is that **modifying the condition variable inside the conditional block can lead to confusing code and potential bugs**. After the `if-else` statement completes:
- If the original `a > 0` was true, `a` will now be 5 (not its original value)
- If the original `a > 0` was false, `a` remains unchanged

This pattern is generally considered **bad practice** because:
1. It makes the code harder to reason about
2. The condition check and the modification are coupled
3. Someone reading the code might not realize `a` changes value
4. It could lead to unexpected behavior if the code is extended later

**Better alternatives:**
- Use a different variable for the assignment
- Restructure the logic to avoid modifying condition variables
- Make the intent clearer with comments if modification is truly necessary
