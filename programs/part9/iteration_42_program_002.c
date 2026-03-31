// Using function attributes (cleaner syntax)
void foo() __attribute__((optimize("O0"))) {
    /* complex code */
}

// Using compiler flags for specific files
// gcc -O0 foo.c -c  // Compile just this file without optimizations
