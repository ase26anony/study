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
    /* Pattern: while loop with decrement and zero comparison */
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
    /* Pattern: while with post-decrement comparison */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while(int limit) {
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
int test_nested_loop(int limit) {
    int sum = 0;
    /* Pattern: nested loops with inner decrementing counter */
    for (int j = 0; j < 3; j++) {
        for (int i = limit; i > 0; i--) {
            sum += i + j;
        }
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total = 0;
    
    /* Call all test functions with volatile bound */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_while_postdec(iterations);
    total += test_do_while(iterations);
    total += test_unsigned_loop((unsigned int)iterations);
    total += test_nested_loop(iterations / 10);
    
    printf("Total sum: %d\n", total);
    
    /* Also test with random value to avoid pattern recognition */
    srand(42);
    int random_iter = rand() % 1000 + 1;
    total += test_for_loop(random_iter);
    
    return total > 0 ? 0 : 1;
}
