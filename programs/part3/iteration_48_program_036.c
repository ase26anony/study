## **Key Insight:**
The `val` variable in your original code becomes **two different SSA variables**:
- `val_A` (assigned in Path A)
- `val_B` (assigned in Path B)

These are merged by a phi node into `val_1`, which is what gets compared in `val == 0`.

## **Compiler Optimization Opportunity:**
A smart compiler can analyze this and see that:
- When `some_condition(i)` is true → `val_1 = 1` → `val_1 == 0` is false
- When `some_condition(i)` is false → `val_1 = 0` → `val_1 == 0` is true

So the condition `val_1 == 0` is **equivalent to `!some_condition(i)`**, and the compiler could potentially optimize the code to:
