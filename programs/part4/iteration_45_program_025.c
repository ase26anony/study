retry:
    if (cond) {
        // do something
        goto retry;  // This would create a loop
    }
