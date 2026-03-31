// Compile with -O2 normally
void fast_function() { /* optimized code */ }

#pragma GCC push_options
#pragma GCC optimize("O0")
void debug_function() { 
    // This code won't be optimized
    // Variables won't be removed or reordered
    // Easier to debug
}
#pragma GCC pop_options

void another_function() { /* back to -O2 optimization */ }
