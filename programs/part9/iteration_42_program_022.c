// Normal code with optimizations
void optimized_function() {
    // Compiler will optimize this heavily
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex debugging code
    // Variables won't be optimized away
    // Code order preserved exactly as written
    volatile int debug_counter = 0;  // Won't be optimized
    // ... complex logic
}
#pragma GCC pop_options

// Back to normal optimizations
void another_optimized_function() {
    // Compiler optimizations resume here
}
