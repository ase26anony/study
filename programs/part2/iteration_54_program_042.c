This code is **safe** and does **not** have undefined behavior.  

Here’s why:

1. **The condition `x > 0`** reads the value of `x` (which is 5).  
2. **Inside the `if` block**, `x` is assigned a new value (`10`).  
3. **C and C++ rules** allow modifying a variable in the same statement where it’s used in the controlling expression, as long as the modification happens **after** the value is read for the condition.  

The sequence is well-defined:  
- First, evaluate `x > 0` → `true`.  
- Then, enter the block and execute `x = 10`.  

No undefined behavior occurs here.  

**Undefined behavior would occur** if you modified `x` in the same expression where it’s read without a sequence point in between (e.g., `if (x++ > 0) { x = 10; }` is fine, but something like `if (x = x + 1)` is fine too — just different). Actually, let's clarify: modifying `x` in the controlling expression itself (like `if (++x > 0)`) is fine because the increment is completed before entering the body. UB happens in cases like `if (x++ > x)` where the same variable is read and written in the same expression without sequencing.  

In your example, the read (`x > 0`) and the write (`x = 10`) are **separated by a sequence point** after the full expression of the condition and before the body runs, so it’s safe.
