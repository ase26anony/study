Looking at this code, the key insight is that `x & 0xFC` (where `0xFC` is binary `11111100`) will clear the lowest 2 bits of `x`, making `y` a multiple of 4.

Let's analyze the possible values of `y` in both branches:

**Branch 1 (cond is true):**
- `x = some_value % 100` → `x ∈ [0, 99]`
- `y = x & 0xFC` → `y ∈ {0, 4, 8, ..., 96}` (multiples of 4 from 0 to 96)

**Branch 2 (cond is false):**
- `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`
- `y = x & 0xFC` → `y ∈ {200, 204, 208, ..., 296}` (multiples of 4 from 200 to 296)

So after the merge point, `y` can be:
- Any multiple of 4 from 0 to 96 (inclusive)
- Any multiple of 4 from 200 to 296 (inclusive)

**Important note:** The value 100 is NOT possible for `y` because:
- In branch 1: `x` max is 99, so `y` max is 96
- In branch 2: `x` min is 200, so `y` min is 200

The range [100, 199] is completely excluded from possible `y` values.
