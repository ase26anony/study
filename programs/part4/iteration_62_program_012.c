int x = phi_result;
int y = x;  // Can be eliminated
int z = x;  // Can be simplified to z = x
if (x == 0) { ... }  // After full propagation
