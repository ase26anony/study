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
    /* Pattern: while loop with decrement and compare to zero */
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
    /* Pattern: while with post-decrement compare */
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
    /* Unsigned version - may generate different but valid pattern */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int iterations = 100;
    int total = 0;
    
    /* Call all test functions to exercise different loop patterns */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_while_postdec(iterations);
    total += test_do_while(iterations);
    total += test_unsigned_loop(iterations);
    
    printf("Total sum: %d\n", total);
    
    /* Also test with random value to avoid complete optimization */
    srand(42);
    int random_iter = rand() % 1000 + 1;
    printf("Random test result: %d\n", test_for_loop(random_iter));
    
    return 0;
}
