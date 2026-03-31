// Compile rest of file with -O2
void normal_function() {
    // Optimized with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    volatile int x = 0;  // Won't be optimized away
    // Hardware access or timing-sensitive code
}
#pragma GCC pop_options

void another_function() {
    // Back to -O2 optimization
}
