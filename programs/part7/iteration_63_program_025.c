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
    return sum + i;  /* i is 0 at exit, but ensures i is live */
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
    
    return sum + (int)u;  /* Ensure u is used */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Initialize counter */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure loop executes at least once */
        do {
            sum += 3;
        } while (--count);  /* Pre-decrement in condition */
    }
    
    return sum + count;  /* count is 0 at exit */
}

/* Function 4: Alternative signed loop with explicit register variable */
__attribute__((noinline, noclone))
int test_explicit_register(int n) {
    register int counter asm("r12") = n;  /* Strong hint for register */
    volatile int accumulator = 0;
    
    /* Pattern: for (register int counter = N; counter != 0; counter--) */
    for (; counter != 0; counter--) {
        accumulator += 4;
    }
    
    return accumulator + counter;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_explicit_register(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return success if total matches expected value */
    /* Expected: (100*1 + 0) + (100*2 + 0) + (100*3 + 0) + (100*4 + 0) = 1000 */
    return (total == 1000) ? 0 : 1;
}
