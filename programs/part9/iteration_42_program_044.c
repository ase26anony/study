// Normal optimized code
void fast_function() {
    // ... optimized code ...
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    volatile int x = 0;
    for(int i = 0; i < 1000; i++) {
        x += i * i;  // Won't be optimized away
    }
}
#pragma GCC pop_options

// Back to normal optimizations
void another_fast_function() {
    // ... optimized code ...
}
