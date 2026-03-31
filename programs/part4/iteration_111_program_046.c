if (cond > 0) {  // Hoisted check
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}
