int val;
if (input > 10) {
    val = some_computation();  // First assignment to 'val'
} else {
    val = another_computation();  // Second assignment to 'val' - violates SSA!
}
if (val == 0) { ... }
