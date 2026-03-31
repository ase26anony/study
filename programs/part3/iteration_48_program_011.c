val_initial = 0
for i = 0 to n-1:
    if (some_condition(i)):
        val_A = 1
    else:
        val_B = 0
    val_phi = φ(val_A, val_B)  // phi node merging both paths
    if (val_phi == 0):
        // do work
