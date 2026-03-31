int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    // Phi node merges values from previous iteration and initial value
    int val_phi = φ(val_0, val_2);  // Phi at start of loop body
    
    if (some_condition(i)) {
        int val_1 = 1;  // One incoming value to Phi
    } else {
        int val_2 = 0;  // Other incoming value to Phi
    }
    
    // Another Phi to merge val_1 and val_2
    int val_3 = φ(val_1, val_2);
    
    if (val_3 == 1) {  // This is the pattern: SSA_NAME (val_3) from Phi compared to constant 1
        // do work
    }
}
