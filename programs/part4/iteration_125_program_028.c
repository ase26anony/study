Looking at this code, the key insight is that after the `if-else` block, `x` can be in one of two ranges:
- `[0, 100)` when `cond` is true
- `[200, 300)` when `cond` is false

When we apply `y = x & 0xFC` (which is `0xFC = 252` in decimal), we need to understand what happens in both cases.

## Analysis:

### Case 1: `cond` is true, `x ∈ [0, 100)`
- `x` is between 0 and 99 inclusive
- `0xFC` in binary is `11111100` (keeps upper 6 bits, clears lower 2 bits)
- For `x < 100`, the AND operation just clears the lowest 2 bits
- Result: `y ∈ {0, 4, 8, 12, ..., 96}` (multiples of 4 up to 96)

### Case 2: `cond` is false, `x ∈ [200, 300)`
- `x` is between 200 and 299 inclusive
- Let's look at the binary representation:
  - 200 in binary: `11001000`
  - 299 in binary: `100101011` (but actually 299 needs 9 bits)
- The AND with `0xFC` will clear the lowest 2 bits
- Since `x ≥ 200`, the result will be `≥ 200`

### Key observation:
The AND operation with `0xFC` clears the lowest 2 bits, which means:
- It rounds down to the nearest multiple of 4
- It doesn't affect whether `x` is in `[0, 100)` or `[200, 300)` because:
  - If `x < 100`, then `x & 0xFC < 100`
  - If `x ≥ 200`, then `x & 0xFC ≥ 200` (since clearing bits can only decrease the value, and 200 is already a multiple of 4: `200 & 0xFC = 200`)

## Result:
After `y = x & 0xFC`:
- If `cond` was true: `y ∈ {0, 4, 8, ..., 96}` (multiples of 4, 0 ≤ y ≤ 96)
- If `cond` was false: `y ∈ {200, 204, 208, ..., 296}` (multiples of 4, 200 ≤ y ≤ 296)

The two ranges remain disjoint because the gap between them (96 to 200) is larger than the maximum possible reduction from the AND operation (at most 3).
