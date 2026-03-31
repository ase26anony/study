// Default optimization (from compiler flags)
void normal_func() { /* optimized normally */ }

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_func() { 
    // Compiled with NO optimizations
    // Easier to debug, variables won't be optimized away
}
#pragma GCC pop_options

#pragma GCC push_options  
#pragma GCC optimize("O3")
void hot_func() {
    // Compiled with maximum optimizations
    // Might be inlined, unrolled, etc.
}
#pragma GCC pop_options
