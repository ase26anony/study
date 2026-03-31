// Normal optimized code
void fast_function() {
    // ... optimized code ...
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    // Hardware register access that shouldn't be optimized away
    volatile int* reg = (volatile int*)0x12345678;
    *reg = 0xAA;
    // ... more sensitive code ...
}
#pragma GCC pop_options

// Back to normal optimizations
void another_fast_function() {
    // ... optimized again ...
}
