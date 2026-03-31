// Normal code compiled with project's default optimization (e.g., O2 or O3)
void normal_function() {
    // Optimized code
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    // Or code that breaks with aggressive optimizations
}
#pragma GCC pop_options

// Back to normal optimization for rest of file
void another_function() {
    // Optimized code again
}
