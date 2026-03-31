This code appears to be part of a comparison function (likely `operator<` or a `compare` method) for a `double_int` class or struct that represents a double-width integer (likely 128-bit on a 64-bit system, or 64-bit on a 32-bit system).

Here's what the code does:

1. **Reference binding**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts (`a.high` and `b.high`) as **unsigned** integers
   - If `a.high` < `b.high`, returns `-1` (indicating `*this` < `b`)
   - If `a.high` > `b.high`, returns `1` (indicating `*this` > `b`)

3. **Low part comparison (signed)**:
   - If the high parts are equal, compares the low parts (`a.low` and `b.low`)
   - If `a.low` < `b.low`, returns `-1`
   - If `a.low` > `b.low`, returns `1`

4. **Implicit return**: If neither condition is met (both high and low parts are equal), the function would return `0` (though not shown in this snippet).

**Key observations:**
- The high part is compared as unsigned, while the low part appears to be compared as signed (based on the variable types)
- This suggests `double_int` might represent a signed 128-bit integer where:
  - `high` is the most significant part (treated as unsigned for proper signed comparison)
  - `low` is the least significant part
- This is a common pattern for implementing signed multi-precision integer comparisons

**Complete function would likely look like:**
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

This code appears to be from GCC or similar compiler infrastructure, where `HOST_WIDE_INT` is a platform-dependent integer type.
