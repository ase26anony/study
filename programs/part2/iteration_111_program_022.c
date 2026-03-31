/* test_main.c - Primary source file with various pragmas and attributes */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level to engage driver decisions */
#pragma GCC optimize("O2")

/* Function with optimization attribute */
__attribute__((optimize("O3"), noinline)) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver processes constructor initialization */
    volatile int marker = 0;
    (void)marker;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_coverage(void) {
    volatile int marker = 1;
    (void)marker;
}

/* Force generation of coverage data */
__attribute__((used))
static void unused_function_for_coverage(void) {
    volatile int x = 0;
    (void)x;
}

/* Main function with compile-time assertion */
int main(void) {
    /* Static assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    int result = compute_factorial(5);
    printf("Factorial of 5 is %d\n", result);
    
    /* Trigger potential warning for -Werror testing */
    int unused_var __attribute__((unused)) = 42;
    
    return (result == 120) ? 0 : 1;
}
