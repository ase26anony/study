### Step 2: Loop entry - create phi node
At the start of the loop body, we need a phi node for `x` because it has multiple reaching definitions:
- From the initial value (first iteration)
- From the previous iteration's value
