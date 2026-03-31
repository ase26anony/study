/* loop-doloop-patterns.c
 * Test program to trigger the specific decrementing loop pattern
 * (compare (plus reg -1) (const_int 0)) in GCC's doloop optimization pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i = n;  /* Hint to keep counter in register */
    volatile int sum = 0; /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1; /* Simple, non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u = n;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2; /* Different constant to distinguish from Function 1 */
    }
    
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3; /* Different constant to distinguish */
        } while (--count);
    }
    
    return sum + count;
}

/* Function 4: Alternative signed decrement with explicit initialization */
__attribute__((noinline, noclone))
int test_alt_signed_decrement(int n) {
    register int j;
    volatile int sum = 0;
    
    /* Pattern: for (j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        sum += j; /* Use counter in body to create dependency */
    }
    
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Use command line argument if provided, otherwise default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions to ensure paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement((unsigned int)iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alt_signed_decrement(iterations);
    
    printf("Total: %d\n", total);
    
    return 0;
}
