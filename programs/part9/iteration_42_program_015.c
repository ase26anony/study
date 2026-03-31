// Normal optimized code
void fast_function() {
    // ... optimized code ...
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    volatile int counter = 0;  // Won't be optimized away
    for(int i = 0; i < 1000; i++) {
        counter += i;
    }
    // Timing-sensitive code
}
#pragma GCC pop_options

// Back to normal optimizations
void another_fast_function() {
    // ... optimized again ...
}
