int val_initial = 0;
for (int i = 0; i < n; ++i) {
    int val_A, val_B;
    
    if (some_condition(i)) {
        val_A = 1;  // Path A
    } else {
        val_B = 0;  // Path B
    }
    
    // Phi node merges values from different paths
    int val_phi = φ(val_A, val_B);  // This is the SSA phi function
    
    if (val_phi == 0) {  // Now using the phi result
        // do work
    }
}
