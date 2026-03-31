/* test_main.c - Main test file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Force dump directory allocation via pragmas */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"

/* Attempt to set dumpdir via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_main"

#pragma GCC diagnostic pop

/* Function with optimization attribute to influence driver decisions */
__attribute__((optimize("O3"), noinline, cold))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through all compilation phases */
    volatile int marker = 0;
    (void)marker;
}

/* Destructor to ensure cleanup happens */
__attribute__((destructor))
static void cleanup_coverage(void) {
    volatile int marker = 1;
    (void)marker;
}

/* Force generation of coverage data */
__attribute__((noinline))
static void coverage_helper(void) {
    volatile int x = 0;
    if (x) {
        /* Unreachable but helps with branch coverage */
        printf("Never printed\n");
    }
}

/* Main function with multiple compilation paths */
int main(int argc, char **argv) {
    int result;
    
    /* Use the function to prevent optimization */
    if (argc > 1) {
        result = compute_factorial(5);
    } else {
        result = compute_factorial(3);
    }
    
    coverage_helper();
    
    /* Compile-time assertion to ensure type sizes */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    printf("Result: %d\n", result);
    return 0;
}
