int attempts = 0;
retry:
if (attempts++ < 3) {
    if (some_operation() == FAILURE) {
        // wait, log, etc.
        goto retry;
    }
} else {
    // too many attempts, handle error
}
