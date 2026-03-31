// NOT a valid optimization:
if (cond > 0) {  // Can't hoist this check!
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}
