// These would raise INVALID exception if a or b is NaN:
if (a != b)  // or a < b, a > b, etc.

// Whereas these macros handle NaN gracefully:
if (isunordered(a, b))  // explicitly checks for NaN
if (!isgreater(c, d))   // returns false, not exception, for NaN
