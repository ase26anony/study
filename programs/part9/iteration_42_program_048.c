// Normal code with global optimization level (e.g., -O2 from command line)
void normal_function() {
    // Compiled with -O2
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex debugging code
    // Compiled with -O0 regardless of global settings
    volatile int x = 0;  // volatile won't be optimized away
    for(int i = 0; i < 1000; i++) {
        x += i;  // Loop won't be optimized or unrolled
    }
}
#pragma GCC pop_options

void another_function() {
    // Back to original optimization level (-O2)
}
