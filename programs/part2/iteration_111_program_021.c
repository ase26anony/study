/* test_target.c - Main test file to trigger dump directory allocation */
#include <stdio.h>
#include <stdlib.h>

/* Pattern A: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern B: Create conditions for error recovery */
#ifdef TEST_ERROR
#error "This error is conditionally triggered"
#endif

/* Pattern C: Use attributes that might affect dump naming */
__attribute__((noinline, cold)) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern D: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through all compilation phases */
    printf("Initializing coverage test...\n");
}

/* Main function with trivial work */
int main(int argc, char **argv) {
    int result;
    
    /* Pattern E: Compile-time assertion to engage driver */
    _Static_assert(sizeof(void*) >= 4, "Pointer size check");
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 0) n = 5;  /* Prevent recursion issues */
        result = compute_factorial(n);
    } else {
        result = compute_factorial(5);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
