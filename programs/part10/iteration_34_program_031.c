Looking at your code snippets, these are floating-point comparison macros from `<math.h>` that handle NaN (Not-a-Number) cases properly:

## 1. `isunordered(a, b)`
- Returns true if **either** `a` or `b` is NaN
- Equivalent to `(isnan(a) || isnan(b))`
- The comment "triggers UNORDERED" refers to the FPU status flag that gets set when comparing NaN values

## 2. `!isgreater(c, d)`
- `isgreater(c, d)` returns `(c) > (d)` but **without** raising the "invalid" exception if either operand is NaN
- `!isgreater(c, d)` means "c is NOT greater than d" (including when either is NaN)
- This could trigger various FPU flags:
  - **UNGE** (unordered or greater than equal) if you think about the inverse
  - **LT** (less than) if both are numbers
  - **UNORDERED** if either is NaN

## 3. `isnan(e)`
- Direct check if `e` is NaN
- Internally, this often uses an unordered comparison like `(e != e)` since NaN ≠ NaN
- Yes, this involves unordered comparison logic internally

## Key points:
- These macros avoid raising floating-point exceptions when comparing with NaNs
- Regular comparisons like `a > b` would raise an "invalid operation" exception if either is NaN
- The "unordered" concept comes from IEEE 754, where NaN comparisons are neither true nor false but "unordered"

The comments suggest this code is analyzing or testing FPU status flags after these operations.
