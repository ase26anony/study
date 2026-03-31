if (cond > 0) {  // Hoist condition check outside loop
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}
