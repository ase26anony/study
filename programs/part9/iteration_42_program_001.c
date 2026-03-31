// Normal optimized code here
void optimized_function() {
    // This gets optimized
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() {
    // Complex logic that's easier to debug without optimizations
    // Variables won't be optimized away
    // Code order won't be rearranged
}
#pragma GCC pop_options

// Back to normal optimizations
void another_optimized_function() {
    // This gets optimized again
}
