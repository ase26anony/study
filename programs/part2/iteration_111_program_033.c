/* test_coverage.c - Main test file to trigger dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use pragmas to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Function with attribute to influence optimization */
__attribute__((optimize("O3"), noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This runs before main, ensuring compilation proceeds */
    printf("Starting coverage test...\n");
}

/* Main function with multiple compilation phases */
int main(int argc, char **argv) {
    int result;
    
    /* Trivial computation to give compiler work */
    if (argc > 1) {
        result = compute_factorial(atoi(argv[1]));
    } else {
        result = compute_factorial(5);
    }
    
    printf("Result: %d\n", result);
    
    /* Static assert to engage driver's error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}

#pragma GCC diagnostic pop
