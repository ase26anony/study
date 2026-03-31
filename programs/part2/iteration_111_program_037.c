/* test_target.c - Main source file with various patterns to trigger dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Use pragma to influence driver decisions */
#pragma GCC optimize("O0")
#pragma GCC push_options
#pragma GCC optimize("O3")

/* Pattern 2: Function with optimization attribute */
__attribute__((optimize("O2"), noinline, cold))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

#pragma GCC pop_options

/* Pattern 3: Constructor that runs before main to ensure full pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures driver processes constructor data */
    printf("Starting coverage test...\n");
}

/* Pattern 4: Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_coverage(void) {
    printf("Coverage test completed.\n");
}

/* Pattern 5: Force warning that could be turned into error with -Werror */
static int unused_variable __attribute__((unused));

/* Pattern 6: Compile-time assertion to engage driver's parsing */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Pattern 7: Multiple functions with different attributes to create varied IR */
__attribute__((hot, always_inline))
static inline int hot_function(int x) {
    return x * 2;
}

__attribute__((cold, noinline))
static int cold_function(int x) {
    return x / 2;
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    printf("Factorial of 5: %d\n", result);
    
    int hot_result = hot_function(result);
    printf("Hot function result: %d\n", hot_result);
    
    int cold_result = cold_function(result);
    printf("Cold function result: %d\n", cold_result);
    
    /* Use the unused variable to prevent warnings */
    unused_variable = 42;
    
    return 0;
}
