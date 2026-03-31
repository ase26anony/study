retry:
    if (connect() == FAILED) {
        if (retries++ < MAX_RETRIES) {
            sleep(1);
            goto retry;  // Jump back to the label
        }
    }
