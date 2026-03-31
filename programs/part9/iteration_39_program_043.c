// After loop peeling optimization:
if (n > 0) {
    // First iteration (i == 0)
    // body
    
    // Remaining iterations
    for (int i = 1; i < n; i++) {
        // i is never 0 here
        i++;
    }
}
