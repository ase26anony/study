Looking at these floating-point comparison macros, I can see they're designed to handle special cases like NaN (Not-a-Number) values properly. Let me explain what each one does:

## `isunordered(a, b)`
- Returns true if **either** `a` or `b` is NaN
- This is the "unordered" case in IEEE 754 floating-point arithmetic
- Regular comparisons like `a < b` or `a == b` are undefined when either operand is NaN

## `!isgreater(c, d)`
- `isgreater(c, d)` returns `(c) > (d)` **and** is false if either operand is NaN
- So `!isgreater(c, d)` is true when:
  - `c ≤ d` (normal case), OR
  - Either `c` or `d` is NaN
- This could trigger "UNGE" (unordered or greater-equal) or similar conditions

## `isnan(e)`
- Directly checks if `e` is NaN
- This involves unordered comparison semantics because:
  - `e == e` is false for NaN values
  - `e != e` is true for NaN values
  - So `isnan(e)` is essentially `e != e`

## Why these macros matter:
Regular relational operators (`<`, `>`, `==`, etc.) have **undefined behavior** when comparing with NaN values in IEEE 754. These macros provide safe alternatives that:
1. Handle NaN values predictably
2. Avoid raising floating-point exceptions
3. Follow IEEE 754 semantics properly

For example, instead of writing `if (a > b)` which could have undefined behavior with NaN, you'd use `if (isgreater(a, b))` which returns false when either is NaN.
