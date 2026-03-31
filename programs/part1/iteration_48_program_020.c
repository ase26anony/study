for (int i = 0; i < 100; ++i) {
    a = 1;  // Moved into delay slot
    if (trigger > i) {
        goto target_label;  // Branch with delay slot
    }
    // Some other code
    continue;
target_label:
    // a = 1;  // Now in delay slot
    counter += a;
}
