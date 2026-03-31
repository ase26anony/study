// These might raise FP exceptions or have unexpected NaN behavior
if (a != a || b != b)  // Like isunordered(a, b) but may raise exceptions
if (!(c > d))          // Like !isgreater(c, d) but may raise exceptions if NaN
if (e != e)            // Common NaN check (isnan(e) is more readable)
