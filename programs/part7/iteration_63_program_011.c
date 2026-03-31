/* loop-doloop-test.c - Test program for doloop optimization pattern matching */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_for_loop(int iterations) {
    register int i;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = iterations; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i in return to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Test 2: Unsigned decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_for_loop(unsigned int iterations) {
    register unsigned int u;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = iterations; u != 0; u--) {
        sum += 2;  /* Different body to distinguish from test 1 */
    }
    
    /* Use u in return to prevent elimination */
    return sum + (u == 0 ? 1 : 0);
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_loop(int iterations) {
    register int n = iterations;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--n); */
    if (n > 0) {  /* Ensure loop executes at least once */
        do {
            sum += 3;  /* Different body to distinguish */
        } while (--n);
    }
    
    /* Use n in return to prevent elimination */
    return sum + (n == 0 ? 1 : 0);
}

/* Test 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed_loop(int iterations) {
    register int counter = iterations;
    volatile int accumulator = 0;
    
    /* Pattern: while (counter-- != 0) */
    while (counter != 0) {
        accumulator += 4;
        counter--;  /* Decrement in body, but still simple pattern */
    }
    
    return accumulator + (counter == 0 ? 1 : 0);
}

int main(void) {
    int total = 0;
    const int base_iterations = 100;
    
    /* Execute all test functions with compile-time constant */
    total += test_signed_for_loop(base_iterations);
    total += test_unsigned_for_loop(base_iterations);
    total += test_do_while_loop(base_iterations);
    total += test_alternative_signed_loop(base_iterations);
    
    /* Print total to ensure all loops are executed */
    printf("Total: %d\n", total);
    
    return 0;
}
