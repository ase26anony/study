### 2. **Loop Header (Phi Node Creation)**
At the loop header (before the `if` statement), SSA needs a phi node because `x` has:
- One incoming value from the previous iteration (or initial value for first iteration)
- One incoming value from within the loop body
