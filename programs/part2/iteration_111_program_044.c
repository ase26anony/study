/* test_coverage.c - Main test file */
/* Pattern: Use multiple compilation units with different attributes */
/* and pragmas to trigger driver dump file allocation */

/* Force dump directory and base name allocation via #pragma */
#pragma GCC option "-dumpdir ./coverage_dumps/"
#pragma GCC option "-dumpbase coverage_test"
#pragma GCC option "-save-temps"

/* Use optimization attributes to influence driver decisions */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Create a warning that can be controlled with -Werror */
int unused_variable;  /* Will generate -Wunused-variable warning */

/* Use constructor attribute to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the compilation goes through all phases */
}

/* Main function with trivial work */
int main(void) {
    int result = compute_factorial(5);
    
    /* Use _Static_assert to engage driver's compile-time checking */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Return success only if factorial(5) = 120 */
    return (result == 120) ? 0 : 1;
}
