/* test_main.c - Main test file to trigger dump directory allocation */

/* Use pragma to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Attempt to set dump options via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_coverage_dumps/"
#pragma GCC option "-dumpbase coverage_test"

#pragma GCC diagnostic pop

/* Function with optimization attribute to influence code generation */
__attribute__((optimize("O3"), noinline, used))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage_test(void) {
    /* This ensures the driver goes through full initialization */
    volatile int marker = 0;
    (void)marker;
}

/* Destructor to match constructor */
__attribute__((destructor))
static void cleanup_coverage_test(void) {
    volatile int marker = 1;
    (void)marker;
}

/* Main function with coverage instrumentation hint */
__attribute__((noinline))
int main(void) {
    /* Force computation to give compiler work */
    int result = compute_factorial(5);
    
    /* Static assert to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Return computed value to prevent optimization */
    return result == 120 ? 0 : 1;
}
