val_initial = 0
for i = 0 to n-1:
    if (some_condition(i)):
        val_A = 1      // Path A
    else:
        val_B = 0      // Path B
    
    val_phi = φ(val_A, val_B)  // Phi node merges values
    
    if (val_phi == 0):
        // do work
