// Example: Debugging a timing-sensitive function
#pragma GCC push_options
#pragma GCC optimize("O0")
void critical_timing_function() {
    // Without O0, compiler might optimize away the delay loop
    for (volatile int i = 0; i < 1000000; i++) {
        // Busy wait
    }
}
#pragma GCC pop_options

// Example: Preventing optimization of memory operations
#pragma GCC push_options  
#pragma GCC optimize("O0")
void memory_barrier() {
    volatile int barrier = 0;
    // Complex memory operations that shouldn't be reordered
}
#pragma GCC pop_options
