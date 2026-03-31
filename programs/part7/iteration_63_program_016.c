/* loop-doloop-test.c
 * Test program to trigger the specific decrementing loop pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    do {
        sum += 3;
    } while (--counter);  /* Pre-decrement in condition */
    
    return sum + counter;
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j;
    volatile int sum = 0;
    
    /* Pattern: for (j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        sum += 4;
    }
    
    /* Use j in calculation to keep it alive */
    return sum * 2 + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Use command line argument if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_alternative_signed(iterations);
    
    /* Print result to prevent elimination of entire program */
    printf("Total: %d\n", total);
    
    return 0;
}
