Looking at this code, I can see that `x` can be in one of two ranges depending on the condition:

1. **When `cond` is true**: `x ∈ [0, 99]` (since `some_value % 100` gives values 0-99)
2. **When `cond` is false**: `x ∈ [200, 299]` (since `200 + (some_value % 100)` gives values 200-299)

After the `if-else`, we have `int y = x & 0xFC;` where `0xFC` in binary is `11111100` (252 in decimal).

## Analysis of the bitwise AND operation:

### For `x ∈ [0, 99]`:
- All values 0-99 have their lower 2 bits cleared by `& 0xFC`
- This results in `y` being multiples of 4: 0, 4, 8, ..., 96
- The maximum value is 96 (since 99 & 0xFC = 96)

### For `x ∈ [200, 299]`:
- Let's look at the binary representation:
  - 200 in binary: `11001000`
  - 299 in binary: `100101011` (but actually 299 = 256 + 43 = `100101011`)
- The `& 0xFC` operation clears the lower 2 bits
- This results in values: 200, 204, 208, ..., 296
- Note: 200 & 0xFC = 200, 299 & 0xFC = 296

## Key observations:
1. **No overlap**: The two ranges for `y` are disjoint:
   - From first branch: `y ∈ {0, 4, 8, ..., 96}`
   - From second branch: `y ∈ {200, 204, 208, ..., 296}`

2. **Pattern**: All resulting `y` values are multiples of 4

3. **Missing values**: Many multiples of 4 between 100-199 and 300+ are not possible

The `& 0xFC` operation effectively rounds `x` down to the nearest multiple of 4 by clearing the two least significant bits.
