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
    /* Pattern: while loop with decrementing counter compared to zero */
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
    /* Pattern: while loop with post-decrement compared to zero */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_with_check(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: do-while with pre-check (might still trigger the pattern) */
    if (i > 0) {
        do {
            sum += i;
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

__attribute__((noinline, noipa))
int test_counter_in_register(int limit) {
    register int i asm ("r12") = limit;  /* Hint to use register */
    int sum = 0;
    /* Pattern: explicit register variable */
    while (i > 0) {
        sum += i;
        i--;
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total = 0;
    
    /* Call all test functions to ensure execution */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_while_postdec(iterations);
    total += test_do_while_with_check(iterations);
    total += test_unsigned_counter((unsigned int)iterations);
    total += test_counter_in_register(iterations);
    
    printf("Total sum: %d\n", total);
    
    /* Also test with random value to avoid predictable patterns */
    srand(42);
    int random_iter = rand() % 1000 + 1;
    total += test_for_loop(random_iter);
    
    return total > 0 ? 0 : 1;  /* Ensure all loops executed */
}
