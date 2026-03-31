Looking at this code, the key insight is that `x & 0xFC` (where `0xFC` is binary `11111100`) will clear the lowest 2 bits of `x`, making `x` a multiple of 4.

Let's analyze both paths:

**Path 1 (cond is true):**
- `x` is in `[0, 100]` (from `some_value % 100`)
- `y = x & 0xFC` will be in `[0, 100]` but only taking multiples of 4
- Maximum possible: `100 & 0xFC = 100 & 0b11111100 = 96` (since 100 in binary is `01100100`, clearing bits 0-1 gives `01100000` = 96)

**Path 2 (cond is false):**
- `x` is in `[200, 299]` (from `200 + (some_value % 100)`)
- `y = x & 0xFC` will be in `[200, 299]` but only taking multiples of 4
- Minimum possible: `200 & 0xFC = 200 & 0b11111100 = 200` (since 200 in binary is `11001000`, clearing bits 0-1 gives `11001000` = 200)
- Maximum possible: `299 & 0xFC = 299 & 0b11111100 = 296` (since 299 in binary is `100101011`, clearing bits 0-1 gives `100101000` = 296)

**Key observation:** The two ranges don't overlap:
- Path 1: `y ∈ {0, 4, 8, ..., 96}`
- Path 2: `y ∈ {200, 204, 208, ..., 296}`

So after the `x & 0xFC` operation, `y` will be:
- Either in `[0, 96]` (multiples of 4) if `cond` was true
- Or in `[200, 296]` (multiples of 4) if `cond` was false

There's a clear gap between 96 and 200, so you can determine which path was taken by looking at `y`:
- If `y < 100`, then `cond` was true
- If `y ≥ 200`, then `cond` was false

This could be useful for constant propagation or range analysis in a compiler optimization pass.
