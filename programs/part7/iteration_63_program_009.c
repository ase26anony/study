/* loop-doloop-test.c
 * Test program to trigger doloop optimization pattern:
 * (compare (plus reg -1) (const_int 0))
 * Targeting uncovered lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i in return to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
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
    
    return sum + (u == 0 ? 1 : 0);
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure loop executes at least once for positive n */
        do {
            sum += 3;
        } while (--counter);
    }
    
    return sum + (counter == 0 ? 1 : 0);
}

/* Function 4: Alternative signed loop with explicit register */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j asm("r12");  /* Stronger register hint */
    volatile int sum = 0;
    int array[10];  /* Small array for indexing */
    
    /* Pattern: for (register int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        /* Use j in array access to create dependency */
        if (j >= 0 && j < 10) {
            array[j % 10] = sum;
        }
        sum += 4;
    }
    
    return sum + (j == 0 ? 1 : 0) + array[0];
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
