/* test_main.c - Primary source file with pragmas and attributes */
#pragma GCC option "-dumpdir ./coverage_dumps/"
#pragma GCC option "-dumpbase coverage_test"
#pragma GCC option "-save-temps=obj"

/* Force dumpbase_ext allocation with multiple input files */
__attribute__((optimize("O3"))) int compute_factorial(int n);

/* Trigger warning to engage error handling with -Werror */
int unused_variable __attribute__((unused)); /* This will generate warning if -Wunused-variable */

/* Constructor to ensure driver processes entire pipeline */
static void __attribute__((constructor)) init_coverage_test(void) {
    /* This runs before main, ensuring compilation proceeds */
}

/* Function with optimization attribute to influence driver decisions */
__attribute__((optimize("O0"), noinline)) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Main function with compile-time assertion */
int main(void) {
    /* Static assertion to engage compiler's error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    int result = compute_factorial(5);
    
    /* Use result to prevent optimization */
    if (result != 120) {
        return 1;
    }
    
    return 0;
}
