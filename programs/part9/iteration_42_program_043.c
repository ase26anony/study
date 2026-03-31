// Main code compiled with -O2
#pragma GCC push_options
#pragma GCC optimize("O0")
void critical_timing_function() {
    // Hardware timing loop or sensitive algorithm
    // Needs exact instruction ordering
    for (volatile int i = 0; i < 1000; i++);
}
#pragma GCC pop_options

// Rest of code continues with -O2 optimization
