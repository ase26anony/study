// Compile whole file with -O2
void normal_func() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_func() {
    // Complex logic that's easier to debug without optimizations
    // Or timing-sensitive code that breaks with optimizations
}
#pragma GCC pop_options

void another_func() {
    // Back to -O2 optimization
}
