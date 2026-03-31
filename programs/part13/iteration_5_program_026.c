if (input > 10) {
    val1 = some_computation();
} else {
    val2 = another_computation();
}
val = φ(val1, val2);  // Phi node merges the two possible values
if (val == 0) { ... }
