This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). The code compares two double integers: `*this` (the current object) and `b` (another double_int object).

Here's what the code does:

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts of the integers as unsigned values
   - If `a.high < b.high`, returns `-1` (meaning `*this < b`)
   - If `a.high > b.high`, returns `1` (meaning `*this > b`)

3. **Low part comparison (unsigned)**:
   - Only reaches this point if the high parts are equal
   - Compares the low parts
   - If `a.low < b.low`, returns `-1`
   - If `a.low > b.low`, returns `1`

4. **Implicit equal case**: If neither high nor low parts differ, the function would return `0` (though not shown in this snippet), meaning the two double integers are equal.

This is a typical implementation for comparing multi-precision integers, where:
- The integer is split into high and low parts (likely each being `HOST_WIDE_INT`, which is typically 64-bit)
- Comparison is done in unsigned mode, which is appropriate for comparing the raw bits of two's complement integers
- The high part is compared first since it's more significant

The function follows the standard comparison convention:
- Returns negative value if `*this < b`
- Returns positive value if `*this > b`  
- Returns 0 if equal

This looks like it could be from GCC or similar compiler infrastructure code.
