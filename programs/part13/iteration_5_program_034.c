int val;  // val is declared but not initialized yet

// Branch point
if (input > 10) {
    val = some_computation();  // val gets value on true path
} else {
    val = another_computation();  // val gets value on false path
}

// Merge point - val could have come from either path
if (val == 0) { ... }  // Uses the merged value
