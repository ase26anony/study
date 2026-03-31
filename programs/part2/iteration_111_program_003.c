/* test_main.c - Primary test file with multiple features to trigger driver dump allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern A: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC push_options
#pragma GCC optimize("O3")

/* Pattern B: Function with attributes that affect compilation */
__attribute__((noinline, cold)) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern C: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver processes constructor initialization */
    volatile int marker = 0;
    (void)marker;
}

/* Pattern D: Multiple compilation units simulated via inline assembly */
#ifdef __x86_64__
__attribute__((target("avx2")))
#endif
static void vector_operation(void) {
    /* Force different optimization contexts */
    volatile int arr[4] = {1, 2, 3, 4};
    (void)arr;
}

/* Main function with compile-time assertions */
int main(void) {
    /* Pattern E: Static assertion to engage type checking */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Pattern F: Trigger warnings that might affect driver behavior */
    int unused_var __attribute__((unused)) = 42;
    
    /* Pattern G: Use computed value to prevent optimization */
    volatile int result = compute_factorial(5);
    
    /* Pattern H: Conditional compilation for error simulation */
#if 0
    /* This syntax error is bypassed but might be parsed */
    int syntax error here;
#endif
    
    printf("Test result: %d\n", result);
    return 0;
}

/* Pattern I: Additional function with different attributes */
__attribute__((hot, optimize("O2")))
static void hot_function(void) {
    for (volatile int i = 0; i < 10; i++) {
        /* Busy work */
    }
}

#pragma GCC pop_options
