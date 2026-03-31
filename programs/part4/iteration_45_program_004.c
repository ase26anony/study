retry:
for (int i = 0; i < 3; i++) {
    if (cond) {
        continue retry;  // Jump to next iteration
    }
}
