This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

## Code Analysis

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts of the integers as unsigned values
   - If `a.high < b.high`, returns `-1` (a < b)
   - If `a.high > b.high`, returns `1` (a > b)

3. **Low part comparison (signed)**:
   - If high parts are equal, compares the low parts
   - If `a.low < b.low`, returns `-1` (a < b)
   - If `a.low > b.low`, returns `1` (a > b)

## Return Values
- `-1`: Current object is less than `b`
- `0`: Current object is equal to `b` (not shown in this snippet, but implied)
- `1`: Current object is greater than `b`

## Interesting Points
- The high part is compared as **unsigned** (`(unsigned HOST_WIDE_INT)`)
- The low part appears to be compared as **signed** (no cast shown)
- This suggests the `double_int` might represent a signed 128-bit integer where the high part contains the sign bit when interpreted as signed

This is likely from GCC or similar compiler code where `HOST_WIDE_INT` is typically `long` or `long long`.
