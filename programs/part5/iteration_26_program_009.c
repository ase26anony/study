This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Let me break down what it's doing:

## What it does:
This is implementing a comparison between two double-width integers (`*this` and `b`). The function returns:
- `-1` if `*this < b`
- `0` if `*this == b` (though 0 isn't shown in this snippet)
- `1` if `*this > b`

## How it works:

1. **Reference binding**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts as **unsigned** integers
   - `(unsigned HOST_WIDE_INT) a.high < (unsigned HOST_WIDE_INT) b.high` returns `-1` if true
   - `(unsigned HOST_WIDE_INT) a.high > (unsigned HOST_WIDE_INT) b.high` returns `1` if true

3. **Low part comparison (only if high parts are equal)**:
   - If the high parts are equal, compares the low parts
   - `a.low < b.low` returns `-1` if true
   - `a.low > b.low` returns `1` if true

## Important notes:
- The high part is compared as **unsigned** while the low part comparison doesn't show an explicit cast (might be unsigned or signed depending on context)
- This is treating the double_int as an **unsigned** integer overall since it starts with unsigned comparison of the high part
- The missing case (returning 0 when both high and low parts are equal) would come after this code snippet

## Typical double_int structure:
Assuming this is from GCC or similar compiler code, `double_int` likely looks like:
```cpp
struct double_int {
    HOST_WIDE_INT low;   // lower half
    HOST_WIDE_INT high;  // upper half
};
```
Where `HOST_WIDE_INT` is typically a 64-bit integer on modern systems, making this a 128-bit integer.
