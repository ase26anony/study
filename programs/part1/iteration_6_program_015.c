This code appears to be part of a comparison function (likely `operator<` or `compare` method) for a `double_int` class or struct that represents a double-width integer. Let me break it down:

## What it does:
This code compares two double-width integers (`a` and `b`) by:
1. First comparing the high parts as **unsigned** integers
2. If the high parts are equal, comparing the low parts as **unsigned** integers
3. Returning -1, 0 (implied), or 1 for less than, equal, or greater than

## Key observations:

1. **`const double_int &a = *this;`** - Creates a reference to the current object for easier access

2. **Unsigned comparison of high parts**: The code casts both `a.high` and `b.high` to `unsigned HOST_WIDE_INT` before comparing. This is important because:
   - It treats the high part as an unsigned value, which affects how negative values are interpreted
   - This suggests the overall comparison is treating the double_int as an **unsigned** double-width integer

3. **Structure**: The function returns:
   - `-1` if `a < b`
   - `1` if `a > b`
   - `0` if `a == b` (implied by reaching the end without returning)

## What's missing:
The code doesn't show the case where both high and low parts are equal, which would return `0`. This is likely handled by the function returning `0` at the end (not shown in your snippet).

## Complete function would look like:
```cpp
int double_int::compare(const double_int &b) const {
  const double_int &a = *this;
  if ((unsigned HOST_WIDE_INT) a.high < (unsigned HOST_WIDE_INT) b.high)
    return -1;
  if ((unsigned HOST_WIDE_INT) a.high > (unsigned HOST_WIDE_INT) b.high)
    return 1;
  if (a.low < b.low)
    return -1;
  if (a.low > b.low)
    return 1;
  return 0;  // They are equal
}
```

This is a common pattern for comparing multi-precision integers, treating them as unsigned values for the comparison.
