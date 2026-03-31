// These raise exceptions with NaN:
if (a != b)  // Raises exception if a or b is NaN
if (c <= d)  // Raises exception if c or d is NaN

// These are safe with NaN:
if (isunordered(a, b))  // True if a or b is NaN
if (!isgreater(c, d))   // True if c <= d OR c/d is NaN
if (isnan(e))           // Direct NaN check
