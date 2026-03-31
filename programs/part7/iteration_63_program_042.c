/* loop-doloop-test.c
 * Test program to trigger the specific decrementing loop pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;  /* i is 0 after loop */
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u is 0 after loop */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_decrement(int n) {
    register int counter = n;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure loop executes at least once */
        do {
            sum += 3;
        } while (--counter);
    }
    
    return sum + counter;  /* counter is 0 after loop */
}

/* Function 4: Alternative signed decrement with explicit initialization */
__attribute__((noinline, noclone))
int test_alt_signed_decrement(int n) {
    register int j = n;  /* Initialize in declaration */
    volatile int sum = 0;
    
    /* Pattern: for (; j != 0; j--) */
    for (; j != 0; j--) {
        sum += 4;
    }
    
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while_decrement(iterations);
    total += test_alt_signed_decrement(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return success if total matches expected value */
    /* Expected: (100*1 + 0) + (100*2 + 0) + (100*3 + 0) + (100*4 + 0) = 1000 */
    if (total == 1000) {
        return 0;
    } else {
        return 1;
    }
}
