if (input > 10) {
    val1 = some_computation();  // First definition
} else {
    val2 = another_computation();  // Second definition
}
// Merge point
val3 = φ(val1, val2)  // Phi function chooses which value to use

if (val3 == 0) { ... }  // Uses the phi result
