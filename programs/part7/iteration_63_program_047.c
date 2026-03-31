/* loop-doloop-test.c - Test program for doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int iterations) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = iterations; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int iterations) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = iterations; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u should be 0 here */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while_decrement(int iterations) {
    register int n = iterations;  /* Initialize counter */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--n); */
    if (n > 0) {  /* Ensure loop executes at least once */
        do {
            sum += 3;
        } while (--n);  /* Pre-decrement and test against 0 */
    }
    
    return sum + n;  /* n should be 0 here */
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int iterations) {
    register int counter = iterations;
    volatile int accumulator = 0;
    
    /* Explicit comparison against 0 */
    while (counter != 0) {
        accumulator += 4;
        counter--;  /* Post-decrement in body */
    }
    
    return accumulator + counter;
}

int main(int argc, char *argv[]) {
    int total = 0;
    int base_iterations = 100;
    
    /* Execute all test functions with different iteration counts */
    total += test_signed_decrement(base_iterations);
    total += test_unsigned_decrement((unsigned int)base_iterations);
    total += test_do_while_decrement(base_iterations);
    total += test_alternative_signed(base_iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return 0;
}
