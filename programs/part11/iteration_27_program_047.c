**Key design elements that target the uncovered code:**

1. **Basic Block Boundaries at Call Sites:**
   - `if (param1 > 100)` branch has a call as the last statement before `return`
   - Loop body ends with a call, loop increment creates new BB
   - Switch cases end with calls before `break`
   - Conditional branches with calls at the end of each branch

2. **Register Pressure:**
   - Many local variables (`var1`-`var10`, `v1`-`v10`)
   - Explicit register variables (`reg_var`, `reg_var2`) bound to call-clobbered registers
   - Variables used across calls forcing save/restore decisions

3. **Call Preservation:**
   - `__attribute__((noinline, noclone))` on helpers
   - `volatile` globals prevent call elimination
   - Results used after calls

4. **Multiple Paths:**
   - Different parameter values exercise different BB structures
   - Loop with varying iterations
   - Switch with multiple cases

**Compilation options to maximize coverage:**
