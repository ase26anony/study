int x = phi_result;
int z = x;  // After copy propagation
if (z == 0) { ... }
