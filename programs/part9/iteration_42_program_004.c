// Compiled with -O2 (from command line or makefile)

#pragma GCC push_options
#pragma GCC optimize("O0")
void sensitive_function() {
    // This will be compiled with -O0
    // Useful for cryptographic operations or timing-sensitive code
}
#pragma GCC pop_options

void normal_function() {
    // This will be compiled with -O2 (the original setting)
}
