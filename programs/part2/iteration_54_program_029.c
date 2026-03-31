This code is **safe** and does **not** cause undefined behavior.  

Let’s break it down:

1. **`int x = 5;`**  
   Declares and initializes `x` to `5`.

2. **`if (x > 0)`**  
   Reads `x` to evaluate the condition. This is fine.

3. **`x = 10;`**  
   Modifies `x` inside the `if` block.

---

**Key point:**  
Modifying a variable inside a block after reading it in the controlling expression is **allowed** in C and C++ (unlike, say, modifying a variable twice without a sequence point in the same expression, which would be UB).  

The sequence here is:
- Read `x` for the `if` condition → value is `5`.
- Condition is true → enter the block.
- Assign `10` to `x`.

No undefined behavior occurs.
