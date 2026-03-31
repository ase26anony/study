/* test_dump_cleanup.c
 * This test triggers allocation of dumpdir, dumpbase, dumpbase_ext, and outbase
 * strings in the GCC driver, ensuring they are freed in driver::finalize.
 */

/* Pattern A: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC target("arch=x86-64")

/* Pattern B: Create conditions for multiple compilation phases */
/* We'll use inline assembly to ensure assembler phase is engaged */
static void __attribute__((used)) force_assembler_phase(void) {
    __asm__ volatile ("nop" : : : "memory");
}

/* Pattern C: Use attributes that affect code generation */
static int __attribute__((optimize("O3"), noinline)) 
compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern D: Constructor to ensure full compilation pipeline */
static void __attribute__((constructor)) init(void) {
    /* This ensures the driver processes constructor data */
    volatile int dummy = 0;
    (void)dummy;
}

/* Pattern E: Destructor to ensure cleanup path is taken */
static void __attribute__((destructor)) cleanup(void) {
    /* Ensure driver completes all phases */
}

/* Pattern F: Use weak symbols to create multiple symbol resolutions */
extern int __attribute__((weak)) weak_symbol(void);
int weak_symbol(void) { return 42; }

/* Pattern G: Create a benign error condition that's bypassed */
#if 0
/* This syntax error is bypassed, but the parser still sees it */
int syntax error here;
#endif

/* Pattern H: Force coverage instrumentation */
#ifdef __COVERAGE__
/* This will be compiled with -fprofile-arcs -ftest-coverage */
volatile int coverage_marker = 1;
#endif

/* Pattern I: Multiple input files strategy - this is file 1 */
/* We'll compile this with another file to trigger dumpbase calculations */
extern int helper_function(void);

/* Main function with trivial work */
int main(void) {
    /* Compute something to give the compiler work */
    int result = compute_factorial(5);
    
    /* Use inline assembly to ensure assembler phase */
    force_assembler_phase();
    
    /* Use weak symbol */
    result += weak_symbol();
    
    /* Call external function from another file */
    result += helper_function();
    
    /* Compile-time assertion to ensure compilation succeeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Return success */
    return (result > 0) ? 0 : 1;
}
