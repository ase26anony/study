Looking at this code, I can see that the condition `cond > 0` is evaluated in every iteration of the loop, but `cond` is being modified inside the loop body. This creates a **loop-variant condition** - the condition's value can change during loop execution.

Here's what happens:

1. **Initial state**: `cond` has some initial value from `some_value()`
2. **Loop invariant analysis**: The condition `cond > 0` is **NOT** a loop invariant because:
   - `cond` is modified inside the loop (`cond = i`)
   - The value of `cond` changes in each iteration where the condition is true
   - Therefore, the condition's truth value can change during loop execution

3. **Potential compiler optimization issue**: A compiler might try to hoist the condition check outside the loop if it appears to be invariant, but in this case, that would be incorrect because:
   - If `cond > 0` is initially true, the compiler might think it stays true
   - But `cond = i` means `cond` becomes the loop index
   - If `i` becomes 0 or negative, the condition becomes false

4. **Actual execution flow**:
   - If initial `cond > 0`, the loop enters the if-block
   - `cond` gets set to `i` (current loop index)
   - On the next iteration, `cond > 0` checks if the previous `i` was > 0
   - This means the loop will continue executing the if-block as long as the previous iteration's index was > 0

**Example**: If initial `cond = 5`:
- Iteration 0: `cond = 5 > 0` → true → `arr[0] = 0`, `cond = 0`
- Iteration 1: `cond = 0 > 0` → false
- Iteration 2-99: `cond = 0 > 0` → false

The loop only executes the if-block in the first iteration (if at all), then stops.

This is a case where a seemingly loop-invariant condition is actually variant due to modification within the loop body, which could potentially confuse optimization passes if they don't perform thorough data flow analysis.
