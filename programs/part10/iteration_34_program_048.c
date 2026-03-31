Looking at these floating-point comparison macros, here are the key points about what each triggers:

## 1. `isunordered(a, b)`
- **Triggers**: UNORDERED
- **Returns true** when either `a` or `b` is NaN (Not a Number)
- This is the **only** macro that specifically tests for unordered comparison results
- Equivalent to: `(a != a) || (b != b)` or `isnan(a) || isnan(b)`

## 2. `!isgreater(c, d)`
- **Triggers**: UNGE (unordered or not greater than or equal)
- `isgreater(c, d)` returns true when `c > d` and neither is NaN
- `!isgreater(c, d)` is true when:
  - `c < d` OR
  - `c == d` OR
  - Either `c` or `d` is NaN (unordered case)
- So it can trigger UNGE (unordered or not greater than or equal)

## 3. `isnan(e)`
- **Triggers**: Unordered comparison (implicitly)
- While `isnan()` itself doesn't perform a comparison, checking for NaN often involves or implies unordered comparisons
- Many implementations of `isnan()` use unordered comparisons internally
- Any code path that depends on `isnan()` will need to handle unordered cases

## Summary of floating-point comparison macros:
- **Ordered comparisons** (return false if either operand is NaN):
  - `isgreater(a, b)` - `a > b`
  - `isgreaterequal(a, b)` - `a >= b`
  - `isless(a, b)` - `a < b`
  - `islessequal(a, b)` - `a <= b`
  - `islessgreater(a, b)` - `a < b || a > b` (but not equal)

- **Unordered-aware comparisons**:
  - `isunordered(a, b)` - true if either is NaN
  - `!isordered(a, b)` - same as `isunordered(a, b)`

The key insight is that **any negation of an ordered comparison** (like `!isgreater()`) will include the unordered case where either operand is NaN, which is why it can trigger UNGE or similar unordered-related conditions.
