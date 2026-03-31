if (cond) {
    val1 = 1;
} else {
    val2 = 2;
}
val3 = φ(val1, val2);  // PHI node: picks val1 if coming from true branch, val2 if from false branch
if (val3 == 1) ...     // Uses the PHI result
