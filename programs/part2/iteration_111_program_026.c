/* test_coverage.c - Main test file */
/* Pattern: Multiple compilation phases with dump options */

/* Use attributes to influence driver decisions */
__attribute__((optimize("O3")))
__attribute__((noinline))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Trigger warning that will be suppressed */
__attribute__((unused))
static int unused_function(void) {
    return 42;
}

/* Constructor to ensure full pipeline processing */
__attribute__((constructor))
static void init_coverage(void) {
    /* Empty - just to ensure constructor processing */
}

/* Main function with trivial work */
int main(void) {
    /* Compute something to give compiler work */
    int result = compute_factorial(5);
    
    /* Use result to avoid optimization */
    if (result != 120) return 1;
    
    return 0;
}
