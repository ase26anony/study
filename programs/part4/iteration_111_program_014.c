// INCORRECT optimization (would change behavior):
if (cond > 0) {
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
} else {
    // Nothing happens
}
