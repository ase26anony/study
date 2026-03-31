int val_initial = 0;
int val_phi;

for (int i = 0; i < n; ++i) {
    // Phi node at start of loop body
    val_phi = (i == 0) ? val_initial : val_prev;
    
    int val_new;
    if (some_condition(i)) {
        val_new = 1;
    } else {
        val_new = 0;
    }
    
    if (val_new == 1) {  // Testing Phi result
        // do work
    }
    
    val_prev = val_new;  // For next iteration's Phi
}
