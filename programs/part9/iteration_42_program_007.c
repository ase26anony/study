// Normal code with optimizations
void optimized_function() {
    // Compiles with project's default optimization (e.g., -O2)
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex algorithm you want to debug
    // Variables won't be optimized away
    // Easier to step through in gdb
}
#pragma GCC pop_options

// Back to normal optimizations
void another_optimized_function() {
    // Compiles with project's default optimization again
}
