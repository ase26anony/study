// Rough equivalents (but these might raise exceptions):
if (a != a || b != b)  // Like isunordered(a, b)
if (!(c > d) || c != c || d != d)  // Like !isgreater(c, d)
if (e != e)  // Like isnan(e)
