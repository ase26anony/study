// Could potentially optimize to:
for (int i = 0; i < n; ++i) {
    if (some_condition(i)) {
        // Path A: val = 1, so condition is false
        // Skip the work
    } else {
        // Path B: val = 0, so condition is true
        // do work directly
    }
}
