int val_initial = 0;
for (int i = 0; i < n; ++i) {
    int val_phi;  // φ node result
    if (some_condition(i)) {
        int val_A = 1;  // Path A definition
        val_phi = val_A;
    } else {
        int val_B = 0;  // Path B definition
        val_phi = val_B;
    }
    // val_phi is the φ node merging val_A and val_B
    if (val_phi == 0) {  // Comparison with constant 0
        // do work
    }
}
