This code appears to be part of a comparison function for a `double_int` class or struct that represents a large integer using two parts: `high` and `low`. Here's what it does:

## Purpose
Compares two `double_int` values (`*this` and `b`) and returns:
- `-1` if `*this < b`
- `1` if `*this > b`
- `0` if `*this == b` (though 0 isn't shown in this snippet)

## How it works

1. **Reference binding**: `const double_int &a = *this` creates a reference to the current object for cleaner code.

2. **High part comparison (unsigned)**:
   - First compares the `high` parts as **unsigned** integers
   - If `a.high < b.high` (unsigned), returns `-1`
   - If `a.high > b.high` (unsigned), returns `1`

3. **Low part comparison (only if high parts are equal)**:
   - If the high parts are equal, compares the `low` parts
   - If `a.low < b.low`, returns `-1`
   - If `a.low > b.low`, returns `1`

## Important detail
The use of `(unsigned HOST_WIDE_INT)` casts for the high part comparison suggests this is treating the integer as an **unsigned** large integer. This is common in systems programming where you need to compare 128-bit (or larger) integers using two 64-bit parts.

## Missing part
The code snippet doesn't show the final `return 0;` case, which would occur when both `high` and `low` parts are equal.

## Complete function would look like:
```cpp
int compare(const double_int& b) const {
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

This is essentially implementing a lexicographic comparison for multi-word integers, treating the high word as more significant.
