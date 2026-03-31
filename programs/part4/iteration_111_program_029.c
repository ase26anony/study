for (int i = 0; i < 100; i++) {
    // cond's value here depends on:
    // 1. Initial value (some_value())
    // 2. Previous iteration's i value (if the if-block executed)
    if (cond > 0) {
        arr[i] = i;
        cond = i;  // This write affects the next iteration's read
    }
}
