// Compile with -O2 globally
#pragma GCC push_options
#pragma GCC optimize("O0")
void critical_function() {
    // Hardware register access that must not be optimized
    volatile int* reg = (volatile int*)0x1234;
    *reg = 1;
    // Complex logic that breaks with optimizations
}
#pragma GCC pop_options

// Other functions still use -O2
void optimized_function() {
    // This gets -O2 optimization
}
