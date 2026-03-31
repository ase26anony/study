/* test_coverage.c - Main test file */
/* Pattern: Multiple input files with different dump options */

/* Use attributes to influence compilation */
__attribute__((optimize("O3")))
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Trigger warning that can be suppressed */
__attribute__((unused))
static int unused_variable;

/* Main function with coverage instrumentation */
int main(void) {
    int result = compute_factorial(5);
    
    /* Use result to avoid optimization */
    if (result != 120) {
        return 1;
    }
    
    return 0;
}
