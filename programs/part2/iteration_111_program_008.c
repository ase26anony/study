/* test_target.c - Main test file to trigger dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use pragmas to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern: Attribute to force different optimization levels */
__attribute__((optimize("O3"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through all compilation phases */
    printf("Initializing coverage test...\n");
}

/* Pattern: Trigger warnings that can be controlled */
__attribute__((unused))
static void unused_function(void) {
    /* This will generate a warning unless suppressed */
}

#pragma GCC diagnostic pop

/* Pattern: Main function with trivial work */
int main(int argc, char **argv) {
    int result;
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0) n = 5;  /* Default */
        result = compute_factorial(n);
    } else {
        result = compute_factorial(5);
    }
    
    printf("Result: %d\n", result);
    
    /* Pattern: Static assert to engage driver's error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
