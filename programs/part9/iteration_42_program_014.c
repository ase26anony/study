// Normal code with optimizations
void normal_function() {
    // Optimized code
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // This code won't be optimized - easier to debug
    volatile int x = 0;
    for(int i = 0; i < 1000; i++) {
        x += i;  // Won't be optimized away
    }
}
#pragma GCC pop_options

// Back to normal optimizations
void another_function() {
    // Optimized again
}
