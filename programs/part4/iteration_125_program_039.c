Looking at this code, the key insight is that `x & 0xFC` (which is 0b11111100 in binary, or 252 in decimal) will clear the lowest 2 bits of `x`, making the result a multiple of 4.

Let's analyze what happens in both branches:

**Branch 1 (cond is true):**
- `x = some_value % 100` → `x ∈ [0, 99]`
- After `y = x & 0xFC`: Since 0xFC = 252, this clears bits 0 and 1
- `y` will be in: {0, 4, 8, 12, ..., 96} (multiples of 4 from 0 to 96)

**Branch 2 (cond is false):**
- `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`
- After `y = x & 0xFC`: 
  - For `x ∈ [200, 255]`: Direct AND operation works normally
  - For `x ∈ [256, 299]`: The AND with 0xFC (252) will clear bits 0 and 1
- `y` will be in: {200, 204, 208, ..., 252, 256, 260, ..., 296} (multiples of 4 from 200 to 296)

**Key observation:**
The value 252 appears in both analyses, but note:
- In branch 1: Maximum possible `y` is 96 (since 99 & 0xFC = 96)
- In branch 2: Minimum possible `y` is 200

So after the AND operation, the ranges are **disjoint**:
- `y ∈ {0, 4, 8, ..., 96}` from branch 1
- `y ∈ {200, 204, 208, ..., 296}` from branch 2

There's a gap between 96 and 200 where no values of `y` can exist. The value 252 can only come from branch 2 (specifically when `x = 252, 253, 254, or 255` since all these give `y = 252` after AND with 0xFC).
