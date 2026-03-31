/* test_dump_cleanup.c
 * This test triggers allocation of dumpdir, dumpbase, dumpbase_ext, and outbase
 * strings in the GCC driver, ensuring they are freed in driver::finalize.
 */

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern 2: Multiple functions with different attributes to create varied compilation units */
__attribute__((noinline)) static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

__attribute__((optimize("O3"))) static int compute_fibonacci(int n) {
    if (n <= 1) return n;
    return compute_fibonacci(n - 1) + compute_fibonacci(n - 2);
}

/* Pattern 3: Constructor to ensure driver processes entire pipeline */
static void __attribute__((constructor)) init_coverage(void) {
    /* This ensures the driver goes through full compilation */
    volatile int dummy = 0;
    (void)dummy;
}

/* Pattern 4: Destructor to ensure cleanup path is taken */
static void __attribute__((destructor)) cleanup_coverage(void) {
    /* Ensure driver completes all phases */
}

/* Pattern 5: Force generation of coverage data */
__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    /* Coverage instrumentation */
    (void)this_fn;
    (void)call_site;
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    /* Coverage instrumentation */
    (void)this_fn;
    (void)call_site;
}

/* Pattern 6: Main function with trivial work */
int main(int argc, char **argv) {
    /* Force computation to ensure compiler doesn't optimize everything away */
    volatile int result1 = compute_factorial(5);  /* 120 */
    volatile int result2 = compute_fibonacci(6);  /* 8 */
    
    /* Use results to prevent dead code elimination */
    if (argc > 1) {
        return result1 + result2;  /* 128 */
    }
    
    return 0;
}

/* Pattern 7: Conditional compilation to create interesting control flow */
#if 0
/* This creates a parsing path that the driver must handle */
int syntax_error_here = ;  /* Missing expression */
#endif

/* Pattern 8: Static assertion to engage type checking */
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");

#pragma GCC diagnostic pop
