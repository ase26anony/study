/* loop-doloop-test.c
 * Test program to trigger GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa))
int test_for_loop(int limit) {
    int sum = 0;
    /* Pattern: for loop with decrementing counter compared to zero */
    for (int i = limit; i > 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while loop with decrement in condition */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: do-while with pre-check */
    if (i > 0) {
        do {
            sum += i;
        } while (--i > 0);
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_unsigned_loop(unsigned int limit) {
    unsigned int sum = 0;
    /* Pattern: unsigned counter with != 0 comparison */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

__attribute__((noinline, noipa))
int test_separate_decrement(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while with separate decrement */
    while (i > 0) {
        sum += i;
        i--;  /* Decrement in body */
    }
    return sum;
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int iterations = 100;
    int total_sum = 0;
    
    /* Call all test functions to exercise different patterns */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_do_while_loop(iterations);
    total_sum += test_unsigned_loop(iterations);
    total_sum += test_separate_decrement(iterations);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Also test with random value to avoid pattern recognition */
    int random_iter = rand() % 1000 + 1;
    total_sum += test_for_loop(random_iter);
    
    return total_sum > 0 ? 0 : 1;
}
