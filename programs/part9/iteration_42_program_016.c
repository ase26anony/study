// Compile entire file with O3 optimization
#pragma GCC optimize("O3")

void optimized_function() {
    // This gets O3 optimization
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // This gets O0 optimization (no optimization)
    // Easier to debug, but slower
}
#pragma GCC pop_options

void another_function() {
    // This gets O3 optimization again (restored by pop_options)
}
