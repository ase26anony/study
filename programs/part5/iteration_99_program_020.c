This code appears to be part of a comparison function for a `double_int` class or structure that represents a large integer using two parts: `high` and `low`. The code compares two `double_int` values (`*this` and `b`) and returns:
- `-1` if `*this` < `b`
- `1` if `*this` > `b`
- `0` if equal (implied by the missing return statement at the end)

Here's a breakdown of what's happening:

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the `high` parts as unsigned integers
   - If `a.high` < `b.high`, returns `-1`
   - If `a.high` > `b.high`, returns `1`

3. **Low part comparison (only if high parts are equal)**:
   - If the high parts are equal, compares the `low` parts
   - If `a.low` < `b.low`, returns `-1`
   - If `a.low` > `b.low`, returns `1`

4. **Implicit equality**: If neither condition is met (high and low parts are equal), the function would return `0`, though that return statement isn't shown in this snippet.

This is a typical pattern for comparing multi-word integers, where you compare the most significant part first, then the least significant part if needed. The use of unsigned comparison for the high part suggests this might be for comparing absolute values or working with unsigned double integers.
