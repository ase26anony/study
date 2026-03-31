/* test_coverage.c - Main test file */
/* Pattern: Use #pragma to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC optimize("O0")

/* Force dump directory/base allocation through compilation flags */
/* This file will be compiled with -save-temps -dumpdir -dumpbase */

/* Include a benign error that gets bypassed to test error recovery */
#if 0
    #error "This error is intentionally bypassed"
#endif

/* Function with attribute to influence optimization dumps */
__attribute__((optimize("O3"), noinline, cold))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This runs before main, ensuring compilation proceeds */
    static volatile int marker = 0;
    marker++;
}

/* Main function with coverage instrumentation */
int main(void) {
    /* Trigger coverage instrumentation */
    int result = compute_factorial(5);
    
    /* Static assert to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Return success */
    return (result == 120) ? 0 : 1;
}

#pragma GCC diagnostic pop
