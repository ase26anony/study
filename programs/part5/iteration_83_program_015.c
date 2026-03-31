This code appears to be part of a comparison function for a `double_int` class or struct that represents a double-width integer (likely 128-bit). Here's what it does:

## Code Analysis

1. **Reference to current object**: `const double_int &a = *this;` creates a reference `a` to the current object for easier access.

2. **High part comparison (unsigned)**: 
   - First compares the high parts of the integers as unsigned values
   - If `a.high < b.high`, returns -1 (a < b)
   - If `a.high > b.high`, returns 1 (a > b)

3. **Low part comparison (unsigned)**:
   - Only reaches this point if high parts are equal
   - Compares the low parts
   - If `a.low < b.low`, returns -1 (a < b)
   - If `a.low > b.low`, returns 1 (a > b)

4. **Implicit return**: If neither condition is met (both high and low parts are equal), the function would return 0 (a == b), though that return statement isn't shown in this snippet.

## Purpose

This implements a standard lexicographic comparison for multi-precision integers:
1. Compare most significant parts first
2. Only compare less significant parts if more significant parts are equal

## Example Structure

The `double_int` class likely looks something like:
```cpp
class double_int {
    HOST_WIDE_INT low;   // Lower half (e.g., 64 bits)
    HOST_WIDE_INT high;  // Upper half (e.g., 64 bits)
    
    int compare(const double_int &b) const {
        // ... the code shown above
    }
};
```

This is a common pattern in compiler codebases (like GCC) for handling wide integers that don't fit in native machine words.
