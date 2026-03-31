// Compile with -O2 globally
void normal_func() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_func() {
    // Complex logic that's easier to debug without optimization
    // Compiled with -O0
}
#pragma GCC pop_options

void another_func() {
    // Back to -O2 optimization
}
