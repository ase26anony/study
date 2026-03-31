/* loop-doloop-test.c
 * Test program to trigger doloop optimization pattern:
 * (compare (plus reg -1) (const_int 0))
 * 
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-test.c
 * Check doloop dump for the pattern in each test function.
 */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;
    }
    
    return sum + (int)u;  /* u should be 0 here */
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_decrement(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    counter = n;  /* Initialize counter */
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            sum += counter;
        } while (--counter);  /* Pre-decrement in condition */
    }
    
    return sum + counter;  /* counter should be 0 here */
}

/* Test 4: Nested decrementing loop to test register pressure handling */
__attribute__((noinline, noclone))
int test_nested_decrement(int n) {
    register int i, j;  /* Multiple register hints */
    volatile int sum = 0;
    
    /* Outer loop with decrement pattern */
    for (i = n; i != 0; i--) {
        /* Inner loop with different counter */
        for (j = 5; j != 0; j--) {
            sum += i * j;
        }
    }
    
    return sum + i + j;  /* Both should be 0 */
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
    total += test_unsigned_decrement((unsigned int)iterations);
    total += test_do_while_decrement(iterations);
    total += test_nested_decrement(iterations / 10);  /* Smaller for nested */
    
    printf("Total: %d\n", total);
    
    return 0;
}
