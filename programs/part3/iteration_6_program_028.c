int val;
if (cond) {
    val = 1;      // val₁ = 1
} else {
    val = 2;      // val₂ = 2
}
// Merge point
if (val == 1) {   // Which val? val₁ or val₂?
    // ...
}
