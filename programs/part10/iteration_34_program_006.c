Looking at these floating-point comparison macros, I can see they're designed to handle special cases like NaN values properly. Let me break down what each one does:

## `isunordered(a, b)`
- Returns true if **either** `a` or `b` is NaN (Not a Number)
- This triggers the "UNORDERED" case when at least one operand is NaN
- Regular comparisons (`<`, `>`, `==`) with NaN always return false, but this macro specifically detects the unordered state

## `!isgreater(c, d)`
- `isgreater(c, d)` returns `(c) > (d)` **without** raising an exception if either operand is NaN
- The negation `!isgreater(c, d)` means:
  - `c <= d` (when both are normal numbers)
  - **OR** either `c` or `d` is NaN
- This could trigger "UNGE" (unordered or greater/equal) or similar conditions

## `isnan(e)`
- Directly checks if `e` is NaN
- This involves unordered comparison semantics because NaN compares unequal to everything, including itself
- `isnan(e)` is equivalent to `e != e` (since NaN ≠ NaN)

## Key points about floating-point comparisons:
1. **Ordered comparisons** (`<`, `>`, `<=`, `>=`) return false if either operand is NaN
2. **Unordered comparisons** detect or handle NaN values explicitly
3. These macros help write robust floating-point code that handles edge cases properly

The "triggers" mentioned in comments likely refer to floating-point exception flags or specific comparison results in low-level floating-point architectures.
