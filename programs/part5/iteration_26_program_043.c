This code appears to be part of a comparison function for a `double_int` class or struct that represents a large integer using two parts: `high` and `low`. Here's what it does:

## Purpose
Compares two `double_int` values (`*this` and `b`) and returns:
- `-1` if `*this < b`
- `1` if `*this > b`
- `0` if equal (though 0 isn't explicitly returned here)

## How it works

1. **Reference binding**: `const double_int &a = *this` creates a reference to the current object for cleaner code.

2. **High part comparison (unsigned)**:
   - First compares the `high` parts as **unsigned** integers
   - If `a.high < b.high`: returns `-1` (current object is smaller)
   - If `a.high > b.high`: returns `1` (current object is larger)

3. **Low part comparison (only if high parts are equal)**:
   - If high parts are equal, compares the `low` parts
   - If `a.low < b.low`: returns `-1`
   - If `a.low > b.low`: returns `1`

## Missing case
The code doesn't explicitly return `0` when both high and low parts are equal. Presumably, there should be a `return 0;` at the end of the function.

## Example
For a `double_int` representing a 128-bit integer:
- `high`: most significant 64 bits
- `low`: least significant 64 bits

The comparison treats the entire number as an unsigned 128-bit integer by comparing the high parts first (as unsigned), then the low parts if needed.
