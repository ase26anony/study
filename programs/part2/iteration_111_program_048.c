/* test_main.c - Primary source file with various directives */
#include <stdio.h>
#include <stdlib.h>

/* Pattern: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern: Use attributes that might affect dump file generation */
__attribute__((noinline)) 
__attribute__((optimize("O3"))) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Constructor to ensure full compilation pipeline */
__attribute__((constructor)) 
static void init_coverage(void) {
    /* This ensures the driver processes constructor initialization */
    volatile int marker = 1;
    (void)marker;
}

/* Pattern: Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_coverage(void) {
    volatile int marker = 0;
    (void)marker;
}

/* Pattern: Force some warnings that can be controlled */
__attribute__((unused))
static void unused_function(void) {
    /* This might generate -Wunused-function unless suppressed */
}

/* Pattern: Use _Static_assert for compile-time checking */
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");

/* Pattern: Main function with trivial work */
int main(int argc, char **argv) {
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0) n = 0;
        int result = compute_factorial(n > 10 ? 10 : n);
        printf("Factorial(%d) = %d\n", n, result);
        return result == 0 ? 1 : 0;
    }
    printf("Usage: %s <number>\n", argv[0]);
    return 0;
}

#pragma GCC diagnostic pop
