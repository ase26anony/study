retry:
    if (cond) {
        // do something
        if (some_error) {
            goto retry;  // Jump back to the label
        }
    }
