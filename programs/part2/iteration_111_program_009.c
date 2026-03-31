/* test_target.c - Primary test file to trigger dumpdir/dumpbase allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern A: Use pragmas to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern D: Attempt to set dumpdir via pragma (may not work but tries) */
/* #pragma GCC option "-dumpdir ./test_dumps/" */

/* Function with attribute to influence optimization dumps */
__attribute__((optimize("O3"), noinline, cold))
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

/* Pattern B: Benign error condition wrapped in #if 0 */
#if 0
/* This would cause a syntax error if enabled */
int bad syntax = error here;
#endif

/* Main function with trivial work */
int main(int argc, char **argv) {
    int result;
    
    if (argc > 1) {
        result = compute_factorial(atoi(argv[1]));
    } else {
        result = compute_factorial(5);
    }
    
    printf("Factorial result: %d\n", result);
    
    /* Pattern C: Compile-time assertion to engage error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}

#pragma GCC diagnostic pop
