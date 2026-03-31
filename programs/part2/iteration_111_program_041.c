/* Test program to trigger GCC driver dump directory allocation and cleanup */
#include <stdio.h>
#include <stdlib.h>

/* Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Force dump file generation through attributes */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through all compilation phases */
    printf("Initializing coverage test...\n");
}

/* Main function with coverage instrumentation hints */
int main(int argc, char **argv) {
    int result;
    
    /* Compile-time assertion to engage driver's parsing */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0) {
            /* Trigger warning that might be caught by -Werror */
            int unused = n;  /* Will trigger -Wunused-but-set-variable if compiled with -Werror */
            (void)unused;    /* Suppress the warning */
        }
        result = compute_factorial(n > 10 ? 10 : n);
    } else {
        result = compute_factorial(5);
    }
    
    printf("Result: %d\n", result);
    
    /* Force generation of coverage data */
    if (result == 120) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}

#pragma GCC diagnostic pop
