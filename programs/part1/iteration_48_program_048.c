if (trigger > i) {
    // Some computation that happens regardless
    b = i + 1;  // Safe in delay slot if b not used before branch
    goto target_label;
}
