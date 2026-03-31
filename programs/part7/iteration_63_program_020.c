/* loop-doloop-patterns.c
 * Test program to trigger the specific decrementing loop pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_for_neq_zero(int n) {
    volatile int sum = 0;  /* Simple volatile accumulator to prevent elimination */
    register int i;        /* Hint to keep counter in register */
    
    /* The critical pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;          /* Minimal non-empty body */
    }
    
    /* Use both counter and accumulator to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_for_neq_zero(unsigned int n) {
    volatile int sum = 0;
    register unsigned int u;  /* Unsigned counter */
    
    /* Unsigned variant: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_predecrement(int n) {
    volatile int sum = 0;
    register int counter = n;  /* Initialize counter */
    
    /* Do-while pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;
        } while (--counter);  /* Critical: --counter in condition */
    }
    
    return sum + counter;
}

/* Main function to execute all tests */
int main(int argc, char **argv) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions with the same iteration count */
    total += test_signed_for_neq_zero(iterations);
    total += test_unsigned_for_neq_zero(iterations);
    total += test_do_while_predecrement(iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return 0;
}
