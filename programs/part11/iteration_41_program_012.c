int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_phi;  // Phi node at start of loop body
    
    if (some_condition(i)) {
        val_1 = 1;  // One incoming value
    } else {
        val_2 = 0;  // Other incoming value
    }
    
    // Phi node merges: val_phi = φ(val_0, val_1, val_2)
    // Actually: val_phi = φ(val_prev, val_1, val_2) where val_prev is from previous iteration
    
    if (val_phi == 1) {
        // do work
    }
}
