You've shown a code snippet where `x` is modified inside an `if` block after being tested in the condition.  

In C and C++, this is **well-defined** — the condition `x > 0` is evaluated once before entering the block, and modifying `x` inside the block doesn’t affect the already-evaluated condition for that execution.  

However, if the condition involved something like `if (x++ > 0)`, then `x` would be modified as part of evaluating the condition itself, which is also valid but requires understanding sequence points to avoid undefined behavior in more complex cases.  

Your example is safe and clear:  

1. `x` is initially `5`.  
2. `x > 0` evaluates to `true`.  
3. Control enters the `if` block and sets `x = 10`.  

No issue here.
