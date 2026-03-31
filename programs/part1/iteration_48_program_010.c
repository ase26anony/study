if (trigger > i) {
    a = 1;  // Executed in delay slot
    goto target_label;
}
// ... rest of code
target_label:
counter += a;  // a is already set to 1 if we branched
