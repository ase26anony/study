val_initial = 0
for i = 0 to n-1:
    if (some_condition(i)):
        val_A = 1      // Path A
    else:
        val_B = 0      // Path B
    
    val_phi = φ(val_A, val_B)  // Phi node merges both paths
    
    if (val_phi == 0):  // Comparison with phi node result
        // do work
