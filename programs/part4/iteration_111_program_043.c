// WRONG optimization - don't do this!
if (cond > 0) {
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
} else {
    for (int i = 0; i < 100; i++) {
        // empty loop - nothing happens
    }
}
