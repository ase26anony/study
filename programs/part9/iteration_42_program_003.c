// Normal optimized code
void fast_function() {
    // ... optimized code ...
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic you want to debug
    // Without optimizations messing with breakpoints
    volatile int x = 0;
    for(int i = 0; i < 1000; i++) {
        x += i * 2;  // Won't be optimized away
    }
}
#pragma GCC pop_options

// Back to normal optimizations
void another_fast_function() {
    // ... optimized code ...
}
