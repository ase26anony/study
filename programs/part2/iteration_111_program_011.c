/* test_target.c - Main test file to trigger dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use pragmas to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern: Function with optimization attribute to affect dump naming */
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

/* Pattern: Trigger a warning that can be controlled */
__attribute__((unused))
static void unused_function(void) {
    /* This may generate -Wunused-function unless suppressed */
}

#pragma GCC diagnostic pop

/* Pattern: Compile-time assertion to engage error handling */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Main function that does trivial work */
int main(int argc, char **argv) {
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0) {
            int result = compute_factorial(n);
            printf("Factorial(%d) = %d\n", n, result);
        }
    }
    return 0;
}
