if (cond > 0) {  // Hoisted outside - WRONG!
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}
