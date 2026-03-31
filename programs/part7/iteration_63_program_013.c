/* loop-doloop-test.c
 * Test program to trigger specific doloop pattern in GCC RTL.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Exact pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i is 0 here, but ensures counter is used */
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Exact pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use counter after loop */
    return sum + (int)u;  /* u is 0 here */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Exact pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            sum += counter;
        } while (--counter);  /* Pre-decrement in condition */
    }
    
    /* Use counter after loop */
    return sum + counter;  /* counter is 0 here */
}

/* Function 4: Alternative signed loop with explicit != 0 */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j;
    volatile int acc = 0;
    
    /* Another variant of the pattern */
    j = n;
    while (j != 0) {
        acc += j;
        j--;  /* Post-decrement in loop body */
    }
    
    return acc + j;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    return 0;
}
