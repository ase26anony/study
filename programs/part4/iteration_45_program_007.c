retry:
    if (connect_to_server() == FAIL) {
        if (retry_count++ < MAX_RETRIES) {
            sleep(1);
            goto retry;  // Jump back to the label
        }
    }
