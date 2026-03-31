/* doloop_test.c - Test program for GCC doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa)) 
int test_for_loop(int limit) {
    int sum = 0;
    /* Classic decrementing for loop compared to zero */
    for (int i = limit; i > 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* While loop with decrement and compare to zero */
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
    /* While loop with post-decrement pattern */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while(int limit) {
    int sum = 0;
    int i = limit;
    
    /* do-while with pre-check - may still trigger pattern matching */
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
    /* Unsigned decrementing loop */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total = 0;
    
    printf("Testing doloop optimization patterns with n = %d\n", iterations);
    
    /* Call all test functions to execute different loop patterns */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_while_postdec(iterations);
    total += test_do_while(iterations);
    total += test_unsigned_loop((unsigned int)iterations);
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Also test with random value to avoid pattern recognition */
    int random_iter = rand() % 1000 + 1;
    printf("Testing with random iteration count: %d\n", random_iter);
    total += test_for_loop(random_iter);
    
    return total != 0 ? 0 : 1;  /* Ensure all loops executed */
}
