/* doloop_test.c - Test program for GCC doloop optimization coverage */

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
    /* Pattern: while loop with explicit decrement */
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
    /* Pattern: while with post-decrement in condition */
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
int test_nested_loop(int outer_limit, int inner_limit) {
    int sum = 0;
    /* Nested loops to test pattern in inner loop */
    for (int i = outer_limit; i > 0; i--) {
        int j = inner_limit;
        while (j > 0) {
            sum += i * j;
            j--;
        }
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total_sum = 0;
    
    /* Call all test functions with different loop patterns */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while(iterations);
    total_sum += test_unsigned_loop((unsigned int)iterations);
    total_sum += test_nested_loop(iterations / 10, iterations / 20);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Also test with random value to avoid pattern recognition */
    srand(42);
    int random_iter = rand() % 1000 + 1;
    total_sum += test_for_loop(random_iter);
    
    printf("Final total: %d\n", total_sum);
    
    return total_sum > 0 ? 0 : 1;  /* Ensure all loops executed */
}
