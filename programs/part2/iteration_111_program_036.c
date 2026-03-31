/* test_coverage.c - Main test file */
/* Pattern: Multiple input files with different base names and extensions */
/* Strategy: Use -save-temps with explicit dumpdir and dumpbase options */

/* Force driver to process pragmas that might affect compilation */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Attempt to set dump options via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./coverage_dumps/"
#pragma GCC option "-dumpbase coverage_test"

#pragma GCC diagnostic pop

/* Use attribute to influence optimization decisions */
__attribute__((optimize("O2"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
static void __attribute__((constructor)) init_coverage_test(void) {
    /* This runs before main, ensuring compilation proceeds */
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Static assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
    
    /* Return success only if computation is correct */
    return (result == 120) ? 0 : 1;
}
