This code appears to be part of a comparison function (likely `operator<` or `compare`) for a `double_int` class/struct that represents a double-precision integer (likely two HOST_WIDE_INT values). Let me break it down:

## What it does:
This function compares two `double_int` values (`*this` and `b`) as **unsigned integers**.

## Structure:
- `double_int` seems to have two members: `high` and `low` (likely representing the high and low parts of a double-width integer)
- The comparison treats the entire value as an unsigned integer by:
  1. First comparing the high parts as unsigned values
  2. If high parts are equal, comparing the low parts

## Return values:
- `-1`: `*this` < `b` (as unsigned)
- `1`: `*this` > `b` (as unsigned)  
- `0`: `*this` == `b` (implied - not shown in this snippet)

## Example:
If this were a 128-bit integer split into two 64-bit parts:
- `high`: most significant 64 bits
- `low`: least significant 64 bits

## Missing part:
The code snippet doesn't show the `return 0;` case which would occur when both `high` and `low` are equal.

## Complete function would likely look like:
```cpp
int compare_unsigned(const double_int &b) const {
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

This is a common pattern for comparing multi-word integers, similar to how we compare multi-digit numbers (compare most significant digit first, then move to less significant digits).
