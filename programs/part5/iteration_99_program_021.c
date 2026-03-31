This code appears to be part of a comparison function (likely `operator<` or `compare`) for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

## Code Analysis

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts (`a.high` and `b.high`) as **unsigned** integers
   - If `a.high` < `b.high`, returns -1 (indicating `*this` < `b`)
   - If `a.high` > `b.high`, returns 1 (indicating `*this` > `b`)

3. **Low part comparison (unsigned)**:
   - Only reaches this point if high parts are equal
   - Compares the low parts (`a.low` and `b.low`) as **unsigned** integers
   - If `a.low` < `b.low`, returns -1
   - If `a.low` > `b.low`, returns 1

## Return Value Convention
- **-1**: Current object (`*this`) is less than `b`
- **0**: Current object equals `b` (not shown in this snippet, but implied)
- **1**: Current object is greater than `b`

## Missing Part
The code snippet doesn't show the final `return 0;` case when both high and low parts are equal, which would complete the comparison function.

## Complete Function Would Look Like:
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
  return 0;  // a == b
}
```

This is a typical implementation for comparing multi-precision integers, treating them as unsigned values.
