/* loop-doloop-test.c
 * Test program to trigger the specific decrementing loop pattern
 * (compare (plus reg -1) (const_int 0)) in GCC's doloop optimization pass.
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
    return sum + i;  /* i is 0 after loop */
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Register hint */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u is 0 after loop */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Initialize counter */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure at least one iteration */
        do {
            sum += 3;
        } while (--count);  /* Pre-decrement in condition */
    }
    
    return sum + count;  /* count is 0 after loop */
}

/* Function 4: Alternative signed loop with explicit register variable */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int counter asm("r12") = n;  /* Strong register hint */
    volatile int accumulator = 0;
    volatile int array[1];  /* Use array to prevent optimizations */
    
    while (counter != 0) {
        array[0] = counter;  /* Use counter in body */
        accumulator += array[0] & 1;  /* Simple operation */
        counter--;  /* Post-decrement */
    }
    
    return accumulator + counter;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int total = 0;
    
    /* Use command line argument if provided, otherwise default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions to ensure code generation */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement((unsigned int)iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (all should return > 0) */
    return (total == 0) ? 1 : 0;
}
