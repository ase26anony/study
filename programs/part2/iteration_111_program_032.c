/* test_dump_cleanup.c
 * This test triggers GCC driver allocation of dump directory and base name strings
 * to cover the cleanup code in driver::finalize (lines 11228-11250 in gcc.cc)
 */

/* Pattern A: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

/* Pattern B: Create conditions for multiple compilation phases */
#include <stdio.h>
#include <stdlib.h>

/* Pattern C: Use attributes that might affect dump file generation */
__attribute__((noinline)) 
__attribute__((optimize("O3"))) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern D: Force coverage instrumentation */
__attribute__((constructor))
static void init_coverage(void) {
    /* This runs before main, ensuring driver processes constructor */
    volatile int marker = 0xDEADBEEF;
    (void)marker; /* Suppress unused warning */
}

/* Pattern E: Create multiple functions with different optimization levels */
__attribute__((optimize("O1")))
static void function_o1(void) {
    volatile int x = 42;
    (void)x;
}

__attribute__((optimize("O2")))
static void function_o2(void) {
    volatile int y = 24;
    (void)y;
}

/* Pattern F: Include inline assembly to engage assembler phase */
static void inline_asm_example(void) {
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        : : : "memory"
    );
}

/* Pattern G: Use static assertion to engage compiler checks */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Main function with trivial work */
int main(int argc, char **argv) {
    /* Pattern H: Conditional compilation to create interesting control flow */
    #ifdef TEST_FLAG
    volatile int flag = 1;
    #else
    volatile int flag = 0;
    #endif
    
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0 && n < 10) {
            int result = compute_factorial(n);
            printf("Factorial(%d) = %d\n", n, result);
        }
    }
    
    function_o1();
    function_o2();
    inline_asm_example();
    
    /* Pattern I: Create a warning that could be promoted to error with -Werror */
    int unused __attribute__((unused)) = 0;
    
    return 0;
}
