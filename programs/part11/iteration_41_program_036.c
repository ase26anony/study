int val_0 = 0;  // Initial value
for (int i = 0; i < n; ++i) {
    int val_i;  // Φ node at start of loop body
    
    // Φ function: val_i = Φ(val_0, val_1, val_2)
    // Where:
    // - val_0 is the value from previous iteration (or initial)
    // - val_1 is the value from "if" branch (1)
    // - val_2 is the value from "else" branch (0)
    
    if (some_condition(i)) {
        int val_1 = 1;  // One incoming value to Φ
    } else {
        int val_2 = 0;  // Other incoming value to Φ
    }
    
    // After the if-else, we have val_i (the result of the Φ node)
    
    if (val_i == 1) {  // This is the pattern you're looking for
        // do work
    }
    
    // For next iteration: val_0 = val_i
}
