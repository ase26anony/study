retry:
    if (condition) {
        // Do something
        if (need_to_retry) {
            goto retry;  // Jump back to the label
        }
    }
