// Pseudo-code showing the dependency
cond_0 = some_value();
for (i = 0; i < 100; i++) {
    if (cond_i > 0) {      // cond_i depends on previous iteration
        arr[i] = i;
        cond_{i+1} = i;    // Sets cond for next iteration
    } else {
        cond_{i+1} = cond_i;  // cond remains unchanged
    }
}
