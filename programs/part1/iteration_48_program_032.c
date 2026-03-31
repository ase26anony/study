for (int i = 0; i < 100; ++i) {
    int temp = 0;  // Default value
    if (trigger > i) {
        temp = 1;  // Moved into delay slot conceptually
        // branch to target_label (but temp already set)
    }
    // Some other code
    continue;
target_label:
    a = temp;  // Use the pre-computed value
    counter += a;
}
