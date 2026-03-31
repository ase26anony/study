Looking at this code, I can see that `x` can be in two different ranges depending on the condition:

1. **When `cond` is true**: `x` is in `[0, 100]` (from `some_value % 100`)
2. **When `cond` is false**: `x` is in `[200, 300]` (from `200 + (some_value % 100)`)

The key insight is that `x & 0xFC` (where `0xFC` = `252` in decimal, or `11111100` in binary) will have different effects depending on which range `x` is in:

## For `x` in `[0, 100]`:
- `0xFC` = `252` (binary: `11111100`)
- Since `x ≤ 100`, `x & 0xFC` will clear the lowest 2 bits of `x`
- The result will be a multiple of 4 in the range `[0, 100]`
- Specifically, `y` will be in: `{0, 4, 8, 12, ..., 96, 100}` (but note 100 & 0xFC = 96)

## For `x` in `[200, 300]`:
- Here we need to consider the binary representation
- `200` in binary: `11001000`
- `300` in binary: `100101100`
- When we do `x & 0xFC`, we're clearing the lowest 2 bits
- The result will be in: `{200, 204, 208, ..., 296, 300}` (but note 300 & 0xFC = 300)

## Important observation:
The operation `x & 0xFC` effectively rounds `x` down to the nearest multiple of 4 by clearing the two least significant bits.

So `y` will be:
- In the range `[0, 96]` (multiples of 4) when `cond` is true
- In the range `[200, 300]` (multiples of 4) when `cond` is false

The two ranges are disjoint because there's a gap between 96 and 200, so you can always tell which branch was taken by looking at the value of `y`.
