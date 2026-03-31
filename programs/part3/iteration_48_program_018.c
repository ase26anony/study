int val;
if (some_condition(i)) {
    val = 1;
} else {
    val = 0;
}
// val now contains either 1 or 0
if (val == 0) {
    // This executes only when some_condition(i) was false
}
