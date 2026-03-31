/* test_dump_cleanup.c
 * This test triggers GCC driver allocation of dump directory and base name strings
 * to cover the cleanup code in driver::finalize (lines 11228-11250 in gcc.cc)
 */

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC optimize("O0")

/* Pattern 2: Create conditions for multiple compilation phases */
#include <stdio.h>
#include <stdlib.h>

/* Function with attribute to influence optimization dump */
__attribute__((optimize("O3"), noinline))
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 3: Use constructor to ensure driver processes entire pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver goes through full compilation */
    volatile int marker = 42;
    (void)marker;
}

/* Pattern 4: Generate warnings that can be controlled */
__attribute__((unused))
static void unused_function(void) {
    /* Trigger -Wunused-function unless suppressed */
    printf("This function is intentionally unused\n");
}

/* Pattern 5: Use static assert to engage type checking */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Main function with multiple compilation-relevant features */
int main(int argc, char **argv) {
    /* Use argc to prevent optimization */
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n > 0 && n < 10) {
            int result = compute_factorial(n);
            printf("Factorial(%d) = %d\n", n, result);
            return result > 0 ? 0 : 1;
        }
    }
    
    /* Default computation */
    int result = compute_factorial(5);
    printf("Default computation: 5! = %d\n", result);
    
    /* Pattern 6: Use inline assembly to engage assembler phase */
    asm volatile ("" : : : "memory");
    
    return 0;
}

#pragma GCC diagnostic pop
