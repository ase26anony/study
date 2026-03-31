int val;
if (input > 10) {
    val = some_computation();  // val1
} else {
    val = another_computation(); // val2
}
// Merge point - phi node needed here
if (val == 0) { ... }
