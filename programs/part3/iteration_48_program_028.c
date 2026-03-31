int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_1, val_2;
    if (some_condition(i)) {
        val_1 = 1;  // Path A
    } else {
        val_2 = 0;  // Path B
    }
    // Phi node merges values from both paths
    int val_3 = φ(val_1, val_2);
    
    if (val_3 == 0) {  // Target condition
        // do work
    }
}
