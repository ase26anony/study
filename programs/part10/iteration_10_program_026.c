/* doloop_test.c - Test program for GCC doloop optimization pattern matching */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa))
int test_for_loop(int limit) {
    int sum = 0;
    /* Pattern: for loop with decrementing counter compared to 0 */
    for (int i = limit; i > 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* Pattern: while loop with decrementing counter compared to 0 */
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
    /* Pattern: while loop with post-decrement compared to 0 */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_with_check(int limit) {
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
int test_unsigned_counter(unsigned int limit) {
    unsigned int sum = 0;
    /* Pattern: unsigned counter with != 0 comparison */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

__attribute__((noinline, noipa))
int test_counter_in_function_arg(int start) {
    int sum = 0;
    int i = start;
    /* Pattern: counter initialized from function argument */
    while (i > 0) {
        sum += (i % 7);  /* Different but simple computation */
        i--;
    }
    return sum;
}

int main(void) {
    volatile int iterations = 100;  /* Prevent constant propagation */
    int total_sum = 0;
    
    /* Call all test functions with non-constant argument */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while_with_check(iterations);
    total_sum += test_unsigned_counter((unsigned int)iterations);
    total_sum += test_counter_in_function_arg(iterations);
    
    /* Also test with random value to prevent pattern recognition */
    int random_iter = rand() % 100 + 50;
    total_sum += test_for_loop(random_iter);
    
    printf("Total sum: %d\n", total_sum);
    return total_sum > 0 ? 0 : 1;  /* Ensure all loops executed */
}
