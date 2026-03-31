/* test_main.c - Main test file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Force dump directory allocation through pragmas */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"

/* Attempt to set dumpdir via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_dumps/"

/* Set dumpbase via pragma */
#pragma GCC option "-dumpbase test_prog"

#pragma GCC diagnostic pop

/* Function with optimization attribute to influence driver decisions */
__attribute__((optimize("O3"), noinline, cold))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through all compilation phases */
    volatile int marker = 42;
    (void)marker;
}

/* Destructor to verify cleanup occurs */
__attribute__((destructor))
static void cleanup_verify(void) {
    /* Verify the program ran */
    volatile int done = 99;
    (void)done;
}

/* Force coverage instrumentation */
__attribute__((noinline))
void coverage_helper(void) {
    /* Empty function to ensure coverage data is generated */
}

/* Main function with various attributes to engage driver */
__attribute__((optimize("O0"), hot))
int main(int argc, char **argv) {
    /* Simple computation to give compiler work */
    int result = compute_factorial(5);
    
    /* Call coverage helper */
    coverage_helper();
    
    /* Use result to prevent optimization */
    if (result != 120) {
        fprintf(stderr, "Unexpected factorial result: %d\n", result);
        return 1;
    }
    
    printf("Test completed successfully. Result: %d\n", result);
    return 0;
}

/* Compile-time assertion to engage error handling */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Additional function with different optimization level */
#pragma GCC optimize("O2")
__attribute__((used))
static void extra_function(void) {
    /* More work for the compiler */
    volatile int x = 0;
    for (int i = 0; i < 100; i++) {
        x += i;
    }
    (void)x;
}
