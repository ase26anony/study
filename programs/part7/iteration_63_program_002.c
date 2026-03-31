/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;  /* Different constant to distinguish from test1 */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            sum += 3;  /* Different constant to distinguish */
        } while (--counter);
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + counter;
}

/* Function 4: Nested decrementing loop to test different context */
__attribute__((noinline, noclone))
int test_nested_decrement(int n) {
    register int i, j;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Outer loop with decrement */
    for (i = n; i != 0; i--) {
        /* Inner loop with different decrement pattern */
        for (j = 5; j != 0; j--) {
            sum += 4;  /* Different constant */
        }
    }
    
    /* Use both counters after loop */
    return sum + i + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_nested_decrement(iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return 0;
}
