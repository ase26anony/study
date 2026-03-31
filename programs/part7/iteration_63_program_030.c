/* loop-doloop-test.c
 * Test program to trigger specific doloop pattern in GCC RTL.
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-test.c
 * Then examine loop-doloop-test.c.234r.doloop for the pattern:
 *   (compare (plus reg -1) (const_int 0))
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
    return sum + (i == 0 ? 1 : 0);
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Register hint */
    volatile int acc = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        acc += 2;
    }
    
    return acc + (u == 0 ? 1 : 0);
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter = n;  /* Must be initialized before loop */
    volatile int total = 0;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure loop executes at least once */
        do {
            total += 3;
        } while (--counter);
    }
    
    return total + (counter == 0 ? 1 : 0);
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j = n;
    volatile int result = 0;
    
    /* Explicit comparison against zero */
    while (j != 0) {
        result += 4;
        j--;  /* Decrement in body, but still separate from comparison */
    }
    
    return result + (j == 0 ? 1 : 0);
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
    total += test_do_while(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    return total > 0 ? 0 : 1;
}
