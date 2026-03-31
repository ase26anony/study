for (int i = 0; i < 100; ++i) {
    if (trigger > i) {
        a = 1;  // Moved into delay slot of branch
        goto target_label;
    }
    // Some other code
    continue;
target_label:
    counter += a;
}
