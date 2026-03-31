/* loop-doloop-test.c
 * Test program to trigger specific uncovered lines in loop-doloop.cc
 * Lines 136-150: Pattern matching for (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u should be 0 here */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_decrement(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    counter = n;  /* Initialize to positive value */
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            sum += 3;
        } while (--counter);  /* Pre-decrement in condition */
    }
    
    return sum + counter;  /* counter should be 0 here */
}

/* Function 4: Alternative signed loop with explicit register */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j asm("r12");  /* Strong hint for specific register */
    volatile int sum = 0;
    int array[10];  /* Small array for indexing */
    
    /* Pattern: for (register int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        /* Use j in array access to create dependency */
        array[j % 10] = j;
        sum += array[j % 10];
    }
    
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;  /* Compile-time constant iteration count */
    int total = 0;
    
    /* Execute all test functions to ensure coverage */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (all should return > 0) */
    return (total == 0) ? 1 : 0;
}
