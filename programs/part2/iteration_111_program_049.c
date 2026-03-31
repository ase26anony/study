/* test_target.c - Main test file to trigger dump directory allocation */
/* Compile with: gcc -O2 -save-temps -dumpdir ./test_dumps/ -dumpbase test_prog \
                  -fprofile-arcs -ftest-coverage test_target.c test_aux.c -o test_target */

/* Use pragma to influence driver behavior */
#pragma GCC optimize("O2")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Function with attribute to influence optimization decisions */
__attribute__((optimize("O3"), noinline, cold))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This forces driver to handle constructor sections */
    volatile int dummy = 0;
    (void)dummy;
}

/* Destructor to ensure cleanup path is taken */
__attribute__((destructor))
static void cleanup_coverage(void) {
    /* Ensure driver completes all phases */
}

/* Main function with multiple compilation paths */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Force multiple optimization decisions */
    if (argc > 1) {
        /* Path 1: Compute factorial */
        result = compute_factorial(5);
    } else {
        /* Path 2: Simple arithmetic */
        result = 42;
    }
    
    /* Use computed result to avoid dead code elimination */
    volatile int use_result = result;
    
    /* Return success */
    return (result > 0) ? 0 : 1;
}

/* Force generation of debug info for additional driver activity */
#pragma GCC diagnostic pop
