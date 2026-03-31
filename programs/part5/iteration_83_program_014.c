This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

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

## Key Observations

1. **Mixed signedness**: The high part is compared as unsigned while the low part appears to be compared as signed (based on the variable names and typical double-int implementations).

2. **Return values**: 
   - `-1` means "less than"
   - `0` would mean "equal" (though not shown in this snippet)
   - `1` means "greater than"

3. **Missing equal case**: The code snippet doesn't show the return for when both high and low parts are equal, which would presumably return `0`.

## Typical Use Case

This is likely implementing a comparison operator (like `operator<` or a `compare` method) for a double-precision integer type, commonly used in compilers (like GCC) for handling large integer constants or intermediate calculations.

## Complete Function

The complete function would probably look like:
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
  return 0;  // equal
}
```
