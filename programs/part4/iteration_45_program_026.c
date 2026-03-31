int attempts = 0;
retry:
if (attempts < 3) {
    attempts++;
    
    // Try some operation
    if (operation_failed) {
        goto retry;  // This jumps back to the label
    }
}
