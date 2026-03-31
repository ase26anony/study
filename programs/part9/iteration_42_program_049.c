// Normal optimized code
void fast_function() {
    // Compiler will optimize this heavily
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // This code won't be optimized - easier to debug
    volatile int x = 0;
    // Complex logic that might behave differently when optimized
}
#pragma GCC pop_options

// Back to normal optimizations
void another_fast_function() {
    // Optimized again
}
