/* doloop_test.c - Test program for GCC doloop optimization pattern matching */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa)) 
int test_for_loop(int limit) {
    int sum = 0;
    /* Pattern: for loop with decrementing counter compared to zero */
    for (int i = limit; i > 0; i--) {
        sum += i * 2;  /* Non-empty but simple body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while loop with explicit decrement and zero comparison */
    while (i > 0) {
        sum += i;
        i--;  /* Decrement by 1 */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_postdec(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while with post-decrement comparison */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_with_check(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: do-while with pre-check (should still trigger pattern matching) */
    if (i > 0) {
        do {
            sum += i * 3;
        } while (--i > 0);
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_unsigned_counter(unsigned int limit) {
    unsigned int sum = 0;
    /* Pattern: unsigned counter with != 0 comparison */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

/* Main function with volatile to prevent constant propagation */
int main(void) {
    volatile int iterations = 100;
    int total_sum = 0;
    
    /* Call all test functions with non-constant argument */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while_with_check(iterations);
    total_sum += test_unsigned_counter((unsigned int)iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    return total_sum > 0 ? 0 : 1;
}
