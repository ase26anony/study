This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

## Code Analysis

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts as unsigned integers
   - If `a.high < b.high`, returns `-1` (a < b)
   - If `a.high > b.high`, returns `1` (a > b)

3. **Low part comparison (signed)**:
   - If high parts are equal, compares the low parts
   - If `a.low < b.low`, returns `-1` (a < b)
   - If `a.low > b.low`, returns `1` (a > b)

## What it returns:
- `-1` if `*this < b`
- `1` if `*this > b`
- `0` if equal (though 0 isn't explicitly returned here - likely handled elsewhere)

## Important Notes:
- The high part is compared as **unsigned** (`(unsigned HOST_WIDE_INT)`)
- The low part appears to be compared as **signed** (no cast shown)
- This suggests the `double_int` might represent a signed 128-bit integer where the high part contains the sign bit when interpreted as signed

## Missing part:
The code doesn't show what happens when both high and low parts are equal. Presumably, there's a `return 0;` statement after this code block, or it's handled in the calling context.

This is a typical pattern for comparing multi-precision integers, comparing from most significant to least significant parts.
