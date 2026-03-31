/* test_dump_cleanup.c
 * This test triggers GCC driver allocation of dump directory and base name strings
 * to cover the cleanup code in driver::finalize (lines 11228-11250 in gcc.cc)
 */

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC target("arch=x86-64")

/* Pattern 2: Create conditions for multiple compilation phases */
#include <stdio.h>
#include <stdlib.h>

/* Pattern 3: Use attributes to force different optimization levels */
__attribute__((optimize("O3"))) 
__attribute__((noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 4: Constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This runs before main, ensuring compilation proceeds */
    volatile int marker = 0;
    (void)marker; /* Suppress unused warning */
}

/* Pattern 5: Trigger warnings that can be controlled */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

/* Pattern 6: Multiple functions with different attributes */
__attribute__((cold))
void cold_function(void) {
    /* Rarely executed code */
    volatile int x = 42;
    (void)x;
}

__attribute__((hot))
int hot_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
    }
    return sum;
}

/* Pattern 7: Assembly integration to force multiple phases */
#ifdef __GNUC__
__asm__(
    ".section .note.GNU-stack,\"\",@progbits\n"
    ".section .note.gnu.property,\"a\"\n"
    ".align 8\n"
    ".long 4\n"
    ".long 0x10\n"
    ".long 0x5\n"
    ".string \"GNU\"\n"
    ".long 0xc0000002\n"
    ".long 4\n"
    ".long 3\n"
    ".align 8\n"
);
#endif

/* Pattern 8: Static assertion to engage type checking */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Pattern 9: Error-like condition wrapped in #if 0 */
#if 0
/* This would cause a compilation error if enabled */
int syntax_error_here missing_semicolon
#endif

/* Pattern 10: Main function with trivial work */
int main(int argc, char **argv) {
    /* Use both hot and cold functions */
    int result = compute_factorial(5);
    hot_function(100);
    cold_function();
    
    /* Print something to avoid dead code elimination */
    if (argc > 1) {
        printf("Factorial of 5 is %d\n", result);
    }
    
    return 0;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
