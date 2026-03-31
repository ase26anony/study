// After copy propagation
if (phi_result == 0) { ... }

// If phi_result is known constant 0 at compile time
if (true) { ... }  // or eliminate the if entirely

// If phi_result is known constant non-zero
if (false) { ... }  // eliminate the if and else block
