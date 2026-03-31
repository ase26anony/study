/* doloop_test.c - Test program for GCC doloop optimization pattern matching */

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
    /* Pattern: while loop with explicit decrement and zero comparison */
    while (i > 0) {
        sum += i;
        i--;
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_postdec(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while loop with post-decrement in condition */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_with_check(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: do-while with pre-check (should still match the comparison pattern) */
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
int test_counter_in_reg(int limit) {
    int sum = 0;
    register int i asm ("r12") = limit;  /* Hint to use specific register */
    /* Pattern: counter in register with decrement and zero comparison */
    while (i > 0) {
        sum += i;
        i = i - 1;  /* Explicit subtraction instead of -- */
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total_sum = 0;
    
    printf("Testing doloop optimization patterns with %d iterations\n", iterations);
    
    /* Call all test functions to exercise different loop patterns */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while_with_check(iterations);
    total_sum += test_unsigned_loop((unsigned int)iterations);
    total_sum += test_counter_in_reg(iterations);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Also test with random value to prevent optimization */
    int random_iter = rand() % 1000 + 1;
    total_sum += test_for_loop(random_iter);
    
    return total_sum != 0 ? 0 : 1;  /* Ensure all loops executed */
}
