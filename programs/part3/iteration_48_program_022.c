int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_1;
    if (some_condition(i)) {
        val_1 = 1;  // Path A
    } else {
        val_1 = 0;  // Path B
    }
    // Phi node: val_2 = φ(val_1, val_0) for next iteration
    // But actually, val_0 is only used for first iteration
    
    if (val_1 == 0) {  // Using val_1 (result of phi from previous iteration)
        // do work
    }
    val_0 = val_1;  // For next iteration (implicit in SSA)
}
