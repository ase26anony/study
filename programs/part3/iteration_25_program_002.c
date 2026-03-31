x1 = φ(x0, x3)  // phi node at loop entry
if (i % 2 == 0)
    x2 = 1
else
    x3 = 0
x4 = φ(x2, x3)  // phi after the if-else
if (x4 == 0)    // Can't be optimized - x4 could be 0 or 1
