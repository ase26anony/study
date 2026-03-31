loop_header:
  val_phi = φ(0, val_prev)  // Phi node: initial 0, then from previous iteration
  // ... loop body sets val_new based on condition
  val_prev = val_new
  goto loop_header
