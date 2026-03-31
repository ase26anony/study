**Step 2: Loop entry - create phi node**
At the start of the loop body, we need a phi node to merge values from:
- The initial value `x₀` (first iteration)
- The previous iteration's value `x₂` (subsequent iterations)
