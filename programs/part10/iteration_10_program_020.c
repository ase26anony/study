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
    /* Pattern: do-while with pre-check (may still trigger the pattern) */
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
int test_counter_in_memory(int limit) {
    int sum = 0;
    int counter = limit;
    /* Pattern: counter in memory location */
    while (counter > 0) {
        sum += counter;
        counter--;
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total_sum = 0;
    
    /* Call all test functions to exercise different loop patterns */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while(iterations);
    total_sum += test_unsigned_loop((unsigned int)iterations);
    total_sum += test_counter_in_memory(iterations);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Also test with random values to avoid pattern recognition */
    for (int j = 0; j < 10; j++) {
        int random_limit = rand() % 50 + 1;
        total_sum += test_for_loop(random_limit);
    }
    
    return total_sum > 0 ? 0 : 1;  /* Ensure all loops executed */
}
