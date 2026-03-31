/* test_target.c - Main test file */
/* Pattern: Use #pragma to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Try to set dump options via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_prog"

#pragma GCC diagnostic pop

/* Use attributes that might affect optimization decisions */
__attribute__((optimize("O2"))) 
__attribute__((noinline))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Force coverage instrumentation */
__attribute__((constructor))
static void init_coverage(void) {
    /* This constructor ensures driver processes the entire pipeline */
    volatile int marker = 0;
    (void)marker; /* Suppress unused warning */
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Compile-time assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
    
    /* Return success only if factorial(5) = 120 */
    return (result == 120) ? 0 : 1;
}
