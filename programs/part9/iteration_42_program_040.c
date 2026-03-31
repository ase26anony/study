// File compiled with -O2 globally
void optimized_function() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's hard to debug when optimized
    // Compiled with -O0 regardless of global settings
}
#pragma GCC pop_options

void another_function() {
    // Back to -O2 optimization
}
