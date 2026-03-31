This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Let me break down what it's doing:

## What it does:
This is implementing a comparison between two double-width integers (`*this` and `b`). The function returns:
- `-1` if `*this < b`
- `1` if `*this > b`
- `0` if `*this == b` (though 0 isn't shown in this snippet)

## How it works:

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts as **unsigned** integers
   - `(unsigned HOST_WIDE_INT) a.high < (unsigned HOST_WIDE_INT) b.high` returns -1 if true
   - `(unsigned HOST_WIDE_INT) a.high > (unsigned HOST_WIDE_INT) b.high` returns 1 if true

3. **Low part comparison (only if high parts are equal)**:
   - If the high parts are equal, then compares the low parts
   - `a.low < b.low` returns -1 if true
   - `a.low > b.low` returns 1 if true

4. **Implicit return 0**: If neither the high nor low comparisons trigger, the numbers are equal, so the function would return 0 (though not shown in this snippet).

## Important detail:
The high parts are compared as **unsigned** integers, which suggests this is implementing an **unsigned comparison** of the full double-width integer. This is a common pattern for comparing multi-precision integers.

## Complete function would likely look like:
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
  return 0;  // equal
}
```

This is from GCC or similar compiler code, where `HOST_WIDE_INT` is typically a 64-bit integer type, making `double_int` a 128-bit integer type.
