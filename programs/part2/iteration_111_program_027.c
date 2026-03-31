/* test_dump_cleanup.c
 * This test triggers GCC driver allocation of dump directory and base name strings
 * to cover the cleanup code in driver::finalize (lines 11228-11250 in gcc.cc)
 */

/* Pattern: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

/* Pattern: Create conditions for dump file generation */
__attribute__((noinline))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Use constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* Force driver to process this file through all stages */
    volatile int marker = 42;
    (void)marker;
}

/* Pattern: Include assembly to force multiple compilation phases */
__asm__(
    ".section .note.GNU-stack,\"\",@progbits\n"
    ".section .rodata\n"
    ".globl asm_data\n"
    "asm_data:\n"
    ".long 0x12345678\n"
);

/* Pattern: Create a benign error condition that gets bypassed */
#if 0
/* This would cause a compilation error if enabled */
int syntax_error_here missing_semicolon
#endif

/* Pattern: Force coverage instrumentation */
__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    /* Empty for coverage */
}

__attribute__((no_instrument_function))
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    /* Empty for coverage */
}

/* Main function with trivial work */
int main(void) {
    /* Pattern: Simple computation to give compiler work */
    int result = compute_factorial(5);
    
    /* Pattern: Use volatile to prevent optimization */
    volatile int output = result;
    
    /* Pattern: Compile-time assertion to engage error handling */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return (output == 120) ? 0 : 1;
}

#pragma GCC diagnostic pop
