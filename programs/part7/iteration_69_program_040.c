a = x * y    // Cycle 1
b = a + z    // Cycle 2 (waits for a)
c = b / 2    // Cycle 3 (waits for b)
if (c > 0)   // Cycle 4 (waits for c)
