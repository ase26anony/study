// Pattern being searched for:
x = phi(...);  // phi node
// ... possibly through some assignments ...
y = x;         // simple copy propagation
if (y == 0)    // or (y == 1)
