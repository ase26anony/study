Looking at this code, the key insight is that `x` can be in one of two ranges:
- **When `cond` is true**: `x ∈ [0, 99]` (since `some_value % 100` gives values 0-99)
- **When `cond` is false**: `x ∈ [200, 299]` (since `200 + (some_value % 100)` gives values 200-299)

After the `if-else`, we compute `y = x & 0xFC` (where `0xFC` = 252 in decimal, or binary `11111100`).

Let's analyze what happens in both cases:

## Case 1: `cond` is true (x ∈ [0, 99])
- `x & 0xFC` will clear the lowest 2 bits of `x`
- Since `x ≤ 99 < 128`, the result will be in `[0, 96]` in steps of 4
- Specifically: `y ∈ {0, 4, 8, ..., 96}`

## Case 2: `cond` is false (x ∈ [200, 299])
- `x & 0xFC` will clear the lowest 2 bits of `x`
- Since `200 ≤ x ≤ 299`, and `0xFC = 252`, the AND operation affects only the lowest 8 bits
- The result will be in `[200, 296]` in steps of 4
- Specifically: `y ∈ {200, 204, 208, ..., 296}`

## Key observation
The operation `x & 0xFC` **doesn't change which range `x` is in** because:
- For `x ∈ [0, 99]`: The highest possible result is 96, which is still < 200
- For `x ∈ [200, 299]`: The lowest possible result is 200, which is still ≥ 200

So after the AND operation:
- If `cond` was true: `y ∈ [0, 96]` (multiples of 4)
- If `cond` was false: `y ∈ [200, 296]` (multiples of 4)

The AND operation simply rounds `x` down to the nearest multiple of 4 within its respective range.
