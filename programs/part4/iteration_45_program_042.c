retry:
    if (cond) {
        // do something
        if (need_to_retry) {
            goto retry;
        }
    }
