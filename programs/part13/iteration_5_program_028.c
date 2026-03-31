if (input > 10) {
    val1 = some_computation();  // First definition
} else {
    val2 = another_computation(); // Second definition
}
// Merge point: phi function combines both definitions
val3 = φ(val1, val2);

if (val3 == 0) { ... }  // Uses the phi result
