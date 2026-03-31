// Original code might have:
if (phi_result == 1)  // cmp_rhs is constant 1
  // branch somewhere

// Where phi_result comes from:
phi_result = phi(0, 1)  // from different incoming edges
