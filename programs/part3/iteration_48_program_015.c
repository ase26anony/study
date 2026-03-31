val_0 = 0
for (i_0 = 0; i_0 < n; i_1 = i_0 + 1) {
    if (some_condition(i_0)) {
        val_1 = 1  // Path A
    } else {
        val_2 = 0  // Path B
    }
    val_3 = φ(val_1, val_2)  // Phi node merging both paths
    if (val_3 == 0) {  // Comparison with constant 0
        // do work
    }
}
