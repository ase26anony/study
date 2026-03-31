int val = 0;
for (int i = 0; i < n; ++i) {
    // Phi node here: val = φ(initial 0, previous val)
    if (some_condition(i)) {
        val = 1;  // Changes val for NEXT iteration
    }
    // Here val could be from Phi (previous iteration) OR from the if above
    if (val == 1) {  // This uses either Phi value or newly assigned value
        // do work
    }
}
