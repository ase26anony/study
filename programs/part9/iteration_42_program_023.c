// Using function attributes (cleaner syntax)
void foo() __attribute__((optimize("O0")));
void foo() { /* complex code */ }

// Using pragmas for the entire file
#pragma GCC optimize("O0")
// All functions here compiled with O0
#pragma GCC reset_options
