/* Main file with various pragmas and attributes to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Attempt to influence dump settings via pragmas */
#pragma GCC option "-dumpdir ./coverage_dumps/"
#pragma GCC option "-dumpbase coverage_test"

#pragma GCC diagnostic pop

/* Function with optimization attribute to influence driver decisions */
__attribute__((optimize("O3"), noinline, used))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage_test(void) {
    /* This ensures the driver goes through all compilation phases */
    volatile int marker = 0;
    (void)marker;
}

/* Main function with coverage instrumentation hints */
__attribute__((noinline))
int main(void) {
    /* Trigger coverage instrumentation */
    int result = compute_factorial(5);
    
    /* Static assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Return success only if computation is correct */
    return (result == 120) ? 0 : 1;
}
