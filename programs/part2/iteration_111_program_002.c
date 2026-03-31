/* test_main.c - Main test file with various GCC pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Force dump directory and base name allocation through pragmas */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"

/* Simulate -dumpdir and -dumpbase via function attributes */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Force generation of auxiliary files */
#pragma GCC option "-fverbose-asm"
#pragma GCC option "-fdump-tree-all"

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through full compilation */
    printf("Initializing coverage test...\n");
}

/* Main function with coverage instrumentation hooks */
int main(int argc, char **argv) {
    int result;
    
    /* Use different optimization levels in different scopes */
    #pragma GCC optimize("O0")
    {
        printf("Testing driver dump directory allocation...\n");
    }
    
    #pragma GCC optimize("O2")
    {
        result = compute_factorial(5);
        printf("Factorial of 5 is: %d\n", result);
    }
    
    /* Trigger warnings that might affect driver behavior */
    #ifdef __COVERAGE__
    int unused __attribute__((unused)) = 42;
    #endif
    
    /* Compile-time assertion to engage driver's type checking */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return result == 120 ? 0 : 1;
}

/* Force dump file generation for this translation unit */
#pragma GCC reset_options
