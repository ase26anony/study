You've shown a code snippet where `x` is modified inside an `if` block after being tested in the condition.  

In C and C++, this is **well-defined** — the condition is evaluated once before entering the block, so changing `x` inside the block doesn’t affect the already-evaluated condition.  

However, if the condition involved something like `if (x++ > 0)`, then `x` would be modified during the evaluation itself, which is a different scenario.  

In your example:  
1. `x > 0` is evaluated (true, since `x == 5`).  
2. Control enters the block.  
3. `x = 10` changes `x`, but the condition is not re-evaluated.  

No undefined behavior here.
