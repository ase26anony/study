// Default optimization (e.g., -O2 from command line)
void optimized_function() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    int x = 5;
    int y = x * 2;  // Won't be optimized away
    // Easier to debug with gdb
}
#pragma GCC pop_options

void another_function() {
    // Back to original optimization level
}
