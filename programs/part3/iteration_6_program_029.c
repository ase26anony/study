// Branch 1
if (cond) {
    val1 = 1;
} else {
    val2 = 2;
}

// Merge point with PHI node
val_phi = φ(val1, val2);  // Selects val1 if coming from true branch, val2 otherwise

// Later use
if (val_phi == 1) {
    // ...
}
