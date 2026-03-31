// Normal code compiled with -O2 or -O3
void fast_function() {
    // Optimized code
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex algorithm that's easier to debug without optimization
    // Variables won't be optimized away during debugging
}
#pragma GCC pop_options

// Back to normal optimization
void another_fast_function() {
    // Optimized again
}
