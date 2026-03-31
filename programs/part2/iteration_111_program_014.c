/* test_target.c - Main test file */
/* Pattern: Use #pragma to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
/* Try to set dump options via pragma - may not work but worth trying */
#pragma GCC option "-dumpdir ./test_dumps/"
#pragma GCC option "-dumpbase test_prog"
#pragma GCC diagnostic pop

/* Use attribute to force optimization level, potentially affecting dump decisions */
__attribute__((optimize("O2"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Constructor to ensure driver processes entire pipeline */
static void __attribute__((constructor)) init_coverage(void) {
    /* This function runs before main, ensuring compilation proceeds */
}

/* Force a warning that we'll treat as error, engaging error recovery */
__attribute__((unused)) 
static int unused_variable;  /* Will generate unused warning with -Werror */

/* Main function with compile-time assertion */
int main(void) {
    /* Static assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Use the function to prevent dead code elimination */
    int result = compute_factorial(5);
    
    /* Return success */
    return result == 120 ? 0 : 1;
}
