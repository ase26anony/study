// Original code might look like:
x = phi(0, 1);  // phi node
// ... possibly some copies: y = x, z = y, etc.
if (z == 0)  // where z is derived from x through copies
