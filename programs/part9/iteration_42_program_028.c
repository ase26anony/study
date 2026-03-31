// Normal code with optimizations enabled
void normal_function() {
    // This gets optimized
    for(int i = 0; i < 1000; i++) {
        // ...
    }
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // This runs without optimizations
    // Easier to debug, variables won't be optimized away
    volatile int counter = 0;  // volatile ensures it's not optimized out
    for(int i = 0; i < 1000; i++) {
        counter++;
        // Complex logic that might behave differently when optimized
    }
}
#pragma GCC pop_options

// Back to normal optimizations
void another_function() {
    // Optimized again
}
