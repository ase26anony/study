int val = 0;
for (int i = 0; i < n; ++i) {
    if (some_condition(i)) {
        val = 1;  // Path A
    } else {
        val = 0;  // Path B
    }
    if (val == 0) {  // Uses phi node result
        // do work
    }
}
