// Compile with -O3 globally
#pragma GCC push_options
#pragma GCC optimize("O0")
void critical_timing_function() {
    // Hardware timing loop or precise delay
    for (volatile int i = 0; i < 1000; i++);
}
#pragma GCC pop_options

// Other functions still use -O3
void optimized_function() {
    // Heavily optimized by compiler
}
