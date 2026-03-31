Looking at this code, the key insight is that `x & 0xFC` (where `0xFC` is binary `11111100`) will clear the lowest 2 bits of `x`, making `x` a multiple of 4.

Let's analyze the possible values of `y` in both branches:

**Branch 1 (cond is true):**
- `x = some_value % 100` → `x ∈ [0, 99]`
- `y = x & 0xFC` → clears lowest 2 bits
- Since `x ≤ 99`, the maximum value after clearing bits is `99 & 0xFC = 96`
- So `y ∈ {0, 4, 8, ..., 96}` (multiples of 4 from 0 to 96)

**Branch 2 (cond is false):**
- `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`
- `y = x & 0xFC` → clears lowest 2 bits
- Since `x ≥ 200`, the minimum value after clearing bits is `200 & 0xFC = 200`
- Since `x ≤ 299`, the maximum value after clearing bits is `299 & 0xFC = 296`
- So `y ∈ {200, 204, 208, ..., 296}` (multiples of 4 from 200 to 296)

**Therefore, the possible values of `y` are:**
- Multiples of 4 from 0 to 96 (inclusive)
- Multiples of 4 from 200 to 296 (inclusive)

**Note:** There's a gap between 96 and 200 where no values of `y` can occur.
