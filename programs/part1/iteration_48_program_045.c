// Pseudo-assembly showing delay slot optimization
for (int i = 0; i < 100; ++i) {
    if (trigger > i) {
        a = 1;        // Moved into delay slot
        goto target_label;
    }
    // Some other code
    continue;
target_label:
    // a = 1; was here originally, now moved up
    counter += a;
}
