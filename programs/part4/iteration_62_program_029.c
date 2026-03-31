int x = phi_result;
int y = x;        // Can be eliminated (y = x)
int z = y;        // Can be eliminated (z = x)
if (z == 0) { ... }
