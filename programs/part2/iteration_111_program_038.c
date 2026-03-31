/* test_target.c - Main test file with dump directory pragmas */
/* This file uses various techniques to force GCC driver to allocate dump directory strings */

/* Pattern: Use #pragma GCC to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Attempt to set dumpdir via pragma (may not work in all GCC versions) */
#pragma GCC option "-dumpdir ./test_coverage_dumps/"

/* Set dumpbase via pragma */
#pragma GCC option "-dumpbase coverage_test"

#pragma GCC diagnostic pop

/* Pattern: Use attributes to influence optimization and dump generation */
__attribute__((optimize("O2"), noinline, cold))
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern: Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage_test(void) {
    /* This will run before main, ensuring compilation proceeds */
    volatile int marker = 42;
    (void)marker; /* Suppress unused warning */
}

/* Pattern: Trigger warnings that can be controlled */
__attribute__((unused))
static void unused_function(void) {
    /* This will trigger -Wunused-function unless we handle it */
}

/* Pattern: Use _Static_assert for compile-time checks */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Pattern: Main function with trivial work */
int main(void) {
    /* Compute something to give compiler work */
    int result = compute_factorial(5);
    
    /* Use result to avoid dead code elimination */
    if (result != 120) {
        return 1; /* Error if factorial is wrong */
    }
    
    /* Additional trivial work */
    volatile int check = result;
    (void)check;
    
    return 0; /* Success */
}

/* Pattern: Include inline assembly to force assembler phase */
__asm__(
    ".section .note.GNU-stack,\"\",@progbits\n"
    ".section .note.gnu.property,\"a\"\n"
    ".align 8\n"
    ".long 4\n"
    ".long 16\n"
    ".long 5\n"
    ".string \"GNU\"\n"
    ".long 0xc0000002\n"
    ".long 4\n"
    ".long 3\n"
    ".align 8\n"
);
