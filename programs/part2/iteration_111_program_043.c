/* Main test file to trigger GCC driver dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use pragmas to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC optimize("O0")

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
    /* This ensures the driver goes through full compilation */
    printf("Initializing coverage test...\n");
}

/* Function with coverage instrumentation */
__attribute__((no_instrument_function))
void coverage_marker(void) {
    /* Empty function for coverage tracking */
}

int main(int argc, char **argv) {
    /* Simple computation to give compiler work */
    int result = compute_factorial(5);
    
    /* Use result to avoid optimization */
    if (result != 120) {
        fprintf(stderr, "Unexpected factorial result: %d\n", result);
        return 1;
    }
    
    printf("Factorial of 5 is: %d\n", result);
    
    /* Trigger coverage recording */
    coverage_marker();
    
    return 0;
}

/* Compile-time assertion to engage driver's error handling */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

#pragma GCC diagnostic pop
