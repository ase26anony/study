// File is compiled with -O2 by default
void bar() {
    // Compiled with O2 optimization
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void foo() {
    // Complex code that breaks with optimizations
    // Compiled with O0 (no optimization)
}
#pragma GCC pop_options

void baz() {
    // Compiled with O2 again (restored by pop_options)
}
