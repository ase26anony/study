/* loop-doloop-test.c
 * Test program to trigger doloop optimization pattern:
 * (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;  /* u should be 0 here */
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--counter); */
    counter = n;
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            sum += counter;
        } while (--counter);  /* Pre-decrement in condition */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + counter;  /* counter should be 0 here */
}

/* Test 4: Another variant with different counter initialization */
__attribute__((noinline, noclone))
int test_another_signed(int n) {
    register int j = n;  /* Initialize in declaration */
    volatile int sum = 0;
    
    /* Pattern: for (register int j = N; j != 0; j--) */
    for (; j != 0; j--) {
        sum += j * 2;  /* Slightly different body */
    }
    
    /* Use j after loop to prevent elimination */
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions to ensure code generation */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_another_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (all should return positive) */
    return (total > 0) ? 0 : 1;
}
