Looking at this code, I can see you're using floating-point comparison macros that handle special cases like NaN (Not a Number) values. Let me break down what each line does:

## 1. `isunordered(a, b)`
- Returns true if **either** `a` or `b` is NaN
- This is the "UNORDERED" case where at least one operand is not a valid number
- The comment is correct - this specifically checks for unordered comparison

## 2. `!isgreater(c, d)`
- `isgreater(c, d)` returns true if `c > d` **and** neither is NaN
- The negation `!isgreater(c, d)` means:
  - `c <= d` (normal case), OR
  - Either `c` or `d` is NaN
- The comment mentions "UNGE" (unordered or not greater than or equal) - this is essentially checking for `c ≤ d` including NaN cases

## 3. `isnan(e)`
- Directly checks if `e` is NaN
- The comment mentions "may involve unordered comparison" - this is because `isnan()` is often implemented using unordered comparisons internally

## Key points about these macros:
- They avoid raising floating-point exceptions when comparing with NaN
- Regular comparisons like `a > b` would raise an invalid operation exception if either operand is NaN
- These macros return `false` (0) when NaN is involved in comparisons (except `isunordered` and `isnan`)

## Equivalent comparisons:
- `isunordered(a, b)` ≡ `(a != a) || (b != b)` (since NaN ≠ NaN)
- `!isgreater(c, d)` ≡ `(c <= d) || isnan(c) || isnan(d)`
- `isnan(e)` ≡ `(e != e)` (the standard way to check for NaN)

These are part of `<math.h>`/`<cmath>` and provide safe NaN-handling comparison operations.
