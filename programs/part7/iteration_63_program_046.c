/* loop-doloop-test.c
 * Test program to trigger the specific decrementing loop pattern
 * (compare (plus reg -1) (const_int 0)) in GCC's doloop optimization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_decrement(int n) {
    register int count = n;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure loop executes at least once */
        do {
            sum += count;
        } while (--count);
    }
    
    /* Use counter in return to prevent elimination */
    return sum + count;
}

/* Main function to execute all tests */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while_decrement(iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return 0;
}
