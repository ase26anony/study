/* loop-doloop-test.c
 * Test program to trigger specific RTL pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Exact pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Register hint */
    volatile int sum = 0;
    
    /* Unsigned variant: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u should be 0 */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Initialize counter */
    volatile int sum = 0;
    
    /* Do-while pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;
        } while (--count);  /* Pre-decrement and test against 0 */
    }
    
    return sum + count;  /* count should be 0 */
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j = n;
    volatile int sum = 0;
    
    /* Another variant that should generate the same pattern */
    while (j != 0) {
        sum += 4;
        j--;  /* Post-decrement in body */
    }
    
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Use command line argument if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (all should return > 0) */
    return (total == 0) ? 1 : 0;
}
