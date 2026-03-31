/* 
 * Main test file to trigger dump directory and base name allocation
 * Uses pragmas and attributes to influence driver behavior
 */

/* Force dump directory and base name allocation via pragmas */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
/* Attempt to set dump options via pragma - may not work in all GCC versions */
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_main"
#pragma GCC diagnostic pop

/* Use optimization attribute to influence driver decisions */
__attribute__((optimize("O3", "no-inline")))
static int compute_factorial(int n) {
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

/* Force generation of coverage data */
__attribute__((noinline))
static void coverage_helper(void) {
    volatile int x = 0;
    (void)x;
}

/* Main function with multiple compilation units */
extern int helper_function(void);
extern void error_simulation(void);

int main(void) {
    /* Trigger coverage instrumentation */
    coverage_helper();
    
    /* Use the optimized function */
    int result = compute_factorial(5);
    
    /* Call external function from another file */
    result += helper_function();
    
    /* Simulate potential error path (compiled out) */
#if 0
    error_simulation();  /* This won't be compiled but influences parsing */
#endif
    
    /* Static assert to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return result == 120 ? 0 : 1;  /* 5! = 120 */
}

/* Force the driver to consider multiple output scenarios */
#ifdef FORCE_DUMP
/* This section only compiled with -DFORCE_DUMP */
#pragma GCC option "-save-temps"
#pragma GCC option "-fverbose-asm"
#endif
