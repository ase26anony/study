/* test_main.c - Main file with dump directory pragmas and attributes */
#include <stdio.h>

/* Use pragma to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Function with optimization attribute to affect codegen */
__attribute__((optimize("O3"), noinline)) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures driver goes through full compilation */
    volatile int marker = 0;
    (void)marker;
}

/* Force some warnings that might affect driver behavior */
__attribute__((unused))
static void unused_function(void) {
    /* Trigger -Wunused-function unless compiled with specific flags */
}

int main(int argc, char **argv) {
    if (argc > 1) {
        int n = 5;
        int result = compute_factorial(n);
        printf("Factorial(%d) = %d\n", n, result);
    }
    return 0;
}

/* Compile-time assertion to engage driver's error handling */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Conditional error that won't trigger */
#if 0
#error "This error is disabled but might influence driver parsing"
#endif
