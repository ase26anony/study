/* test_main.c - Primary test file with various attributes and pragmas */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use attributes to influence optimization decisions */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Use constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through full compilation stages */
    printf("Initializing coverage test...\n");
}

/* Pattern: Trigger warnings that can be controlled */
__attribute__((unused))
static int unused_variable = 42;

/* Pattern: Use _Static_assert for compile-time checks */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Pattern: Try to influence dump options via pragmas */
/* Note: #pragma GCC option is not standard but some compilers accept it */
#ifdef __GNUC__
/* This may influence driver decisions about dump files */
#pragma GCC optimize("O2")
#endif

int main(void) {
    int result = compute_factorial(5);
    printf("Factorial of 5 is: %d\n", result);
    
    /* Pattern: Create a benign warning scenario */
    int x = 10;
    if (x == 10) {
        printf("x is 10\n");
    }
    
    return 0;
}
