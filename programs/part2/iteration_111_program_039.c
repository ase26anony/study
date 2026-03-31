/* test_main.c - Primary test file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern 2: Attribute to force optimization level consideration */
__attribute__((optimize("O3"))) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 3: Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This will execute before main, ensuring compilation proceeds */
    printf("Initializing coverage test...\n");
}

/* Pattern 4: Use static assertion to engage type checking */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Pattern 5: Function with unused variable to potentially trigger warnings */
__attribute__((unused))
static void unused_function(void) {
    int unused_var = 42;
    /* Deliberately unused - may trigger warning unless suppressed */
}

#pragma GCC diagnostic pop

/* Main function that does trivial work */
int main(int argc, char **argv) {
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0) n = 5; /* Default for negative */
        int result = compute_factorial(n > 10 ? 10 : n); /* Limit recursion */
        printf("Factorial(%d) = %d\n", n, result);
    } else {
        printf("Usage: %s <number>\n", argv[0]);
        printf("Factorial(5) = %d\n", compute_factorial(5));
    }
    return 0;
}
