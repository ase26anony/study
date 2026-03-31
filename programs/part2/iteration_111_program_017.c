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

/* Pattern C: Function with attributes that might affect dump naming */
__attribute__((noinline, cold)) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern D: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver processes constructor data */
    printf("Initializing coverage test...\n");
}

/* Main function with optimization attributes */
__attribute__((optimize("O3")))
int main(int argc, char **argv) {
    int result;
    
    /* Simple computation to give compiler work */
    if (argc > 1) {
        result = compute_factorial(atoi(argv[1]));
    } else {
        result = compute_factorial(5);
    }
    
    printf("Factorial result: %d\n", result);
    
    /* Compile-time assertion to engage driver's type checking */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return result == 120 ? 0 : 1;
}

#pragma GCC diagnostic pop
