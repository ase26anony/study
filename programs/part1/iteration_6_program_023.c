This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Let me break down what it's doing:

## Code Analysis

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts as unsigned integers
   - If `a.high < b.high`, returns -1 (a < b)
   - If `a.high > b.high`, returns 1 (a > b)

3. **Low part comparison (signed)**:
   - If high parts are equal, compares the low parts
   - If `a.low < b.low`, returns -1 (a < b)
   - If `a.low > b.low`, returns 1 (a > b)

## Return Value Convention
- Returns -1 if `*this < b`
- Returns 1 if `*this > b`
- Returns 0 if equal (though 0 case isn't shown in this snippet)

## Important Notes

1. **Mixed signedness**: The high part is compared as unsigned while the low part appears to be compared as signed (based on the code shown).

2. **Missing equal case**: The code snippet doesn't show the return 0 case for equality, which would presumably come after all the comparisons.

3. **Typical use**: This is likely part of a `compare()` or `cmp()` method for a big integer class.

## Complete Function Might Look Like:

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
  return 0; // Equal
}
```

This is a common pattern for comparing multi-precision integers where you compare from most significant to least significant parts.
