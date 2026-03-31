// These are approximately equivalent (but may differ in exception behavior):
isunordered(a, b)  ≈  isnan(a) || isnan(b)
isgreater(c, d)    ≈  (c) > (d)  // but no exception for NaN
!isgreater(c, d)   ≈  (c) <= (d) || isnan(c) || isnan(d)
