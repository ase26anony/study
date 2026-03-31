// Main code compiled with -O2
void normal_function() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimization
    // Variables won't be optimized away
    // Code executes exactly as written
}
#pragma GCC pop_options

// Back to -O2 optimization
void another_function() {
    // Compiled with -O2 again
}
