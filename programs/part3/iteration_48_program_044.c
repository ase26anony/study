int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_phi;  // Phi node result
    if (some_condition(i)) {
        int val_A = 1;  // Path A definition
        val_phi = val_A;
    } else {
        int val_B = 0;  // Path B definition
        val_phi = val_B;
    }
    // val_phi is the phi node merging val_A and val_B
    if (val_phi == 0) {  // Using phi result
        // do work
    }
    // Loop back edge: val_0 for next iteration comes from val_phi
}
