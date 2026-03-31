/* loop-doloop-test.c
 * Test program to trigger doloop optimization pattern:
 * (compare (plus reg -1) (const_int 0))
 * Targeting uncovered lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int iterations) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = iterations; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i in return to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int iterations) {
    register unsigned int u;  /* Register hint */
    volatile int accumulator = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = iterations; u != 0; u--) {
        accumulator += 2;
    }
    
    return accumulator + (u == 0 ? 1 : 0);
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter = n;  /* Must be initialized before loop */
    volatile int result = 0;
    
    if (counter <= 0) return 0;
    
    /* Pattern: do { ... } while (--n); */
    do {
        result += 3;
    } while (--counter);  /* Decrement and test against 0 */
    
    return result + (counter == 0 ? 1 : 0);
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int limit) {
    register int j;
    volatile int total = 0;
    
    /* Another variant of the pattern */
    for (j = limit; j != 0; j--) {
        total += j & 1;  /* Simple operation using index */
    }
    
    return total + (j == 0 ? 1 : 0);
}

int main(int argc, char *argv[]) {
    int total = 0;
    int base_iterations = 100;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(base_iterations);
    total += test_unsigned_decrement((unsigned int)base_iterations);
    total += test_dowhile_decrement(base_iterations);
    total += test_alternative_signed(base_iterations);
    
    /* Print result to prevent elimination of entire program */
    printf("Total: %d\n", total);
    
    return total == 0 ? 1 : 0;  /* Non-zero return if all tests ran */
}
