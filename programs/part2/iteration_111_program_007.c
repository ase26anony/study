/* test_main.c - Primary test file with multiple features to trigger dump allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern A: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern B: Attribute that might affect dump naming */
__attribute__((optimize("O3"))) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern C: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver processes constructor initialization */
    printf("Initializing coverage test...\n");
}

/* Pattern D: Multiple functions with different attributes */
__attribute__((noinline, cold))
static void helper_function(void) {
    /* Force separate compilation consideration */
    volatile int x = 42;
    (void)x;
}

/* Pattern E: Error-like condition that's bypassed */
#if 0
/* This would cause a syntax error if compiled, but it's bypassed */
int syntax_error_here = 
#endif

/* Main function with coverage-friendly code */
int main(int argc, char **argv) {
    int result = 0;
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0 && n < 10) {
            result = compute_factorial(n);
            printf("Factorial(%d) = %d\n", n, result);
        }
    } else {
        /* Default computation */
        result = compute_factorial(5);
        printf("Default factorial(5) = %d\n", result);
    }
    
    /* Call helper to ensure it's compiled */
    helper_function();
    
    /* Compile-time assertion to engage driver */
    _Static_assert(sizeof(void*) >= 4, "Pointer size check");
    
    return result > 0 ? 0 : 1;
}

#pragma GCC diagnostic pop
