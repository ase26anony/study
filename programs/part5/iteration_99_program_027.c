This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

## Purpose
This code compares two `double_int` values (`*this` and `b`) and returns:
- `-1` if `*this < b`
- `0` if `*this == b` (though 0 isn't shown in this snippet)
- `1` if `*this > b`

## How it works

1. **Reference creation**: `const double_int &a = *this;` creates a reference to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high 64-bit parts as **unsigned integers**
   - If `a.high < b.high`, returns `-1`
   - If `a.high > b.high`, returns `1`

3. **Low part comparison (signed)**:
   - If the high parts are equal, compares the low 64-bit parts
   - If `a.low < b.low`, returns `-1`
   - If `a.low > b.low`, returns `1`

## Important details

- **Mixed signedness**: The high part is compared as unsigned (`(unsigned HOST_WIDE_INT)`), while the low part appears to be compared as signed (no cast shown for `a.low` and `b.low`).

- **Missing equality case**: The code snippet doesn't show the `return 0;` case, which would occur when both high and low parts are equal.

## Complete function would look like:
```cpp
int compare(const double_int &b) const {
  const double_int &a = *this;
  if ((unsigned HOST_WIDE_INT) a.high < (unsigned HOST_WIDE_INT) b.high)
    return -1;
  if ((unsigned HOST_WIDE_INT) a.high > (unsigned HOST_WIDE_INT) b.high)
    return 1;
  if (a.low < b.low)
    return -1;
  if (a.low > b.low)
    return 1;
  return 0;  // Equal
}
```

This is a common pattern for comparing multi-precision integers, treating them as big-endian (most significant word first).
