/* doloop_test.c
 * Test program to trigger GCC's doloop optimization pattern matching
 * Specifically targets the RTL pattern: (set (reg) (compare (plus (reg) -1) 0))
 */

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
    /* While loop with explicit decrement and zero comparison */
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
    /* While loop with post-decrement in condition */
    while (i-- > 0) {
        sum += i + 1;  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while(int limit) {
    int sum = 0;
    int i = limit;
    /* Do-while with pre-check (might still trigger pattern matching) */
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

__attribute__((noinline, noipa))
int test_counter_in_memory(int limit) {
    int sum = 0;
    int counter = limit;
    /* Force counter in memory initially */
    volatile int* counter_ptr = &counter;
    
    for (int i = *counter_ptr; i > 0; i--) {
        sum += i;
        /* Simple operation, no function calls */
        if (sum > 1000000) sum = 1000000; /* Prevent overflow without complex ops */
    }
    return sum;
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int iterations = 100;
    int total_sum = 0;
    
    /* Call all test functions with non-constant argument */
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdec(iterations);
    total_sum += test_do_while(iterations);
    total_sum += test_unsigned_loop((unsigned int)iterations);
    total_sum += test_counter_in_memory(iterations);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Also test with random value to avoid pattern recognition */
    srand(42);
    int random_iter = rand() % 1000 + 1;
    total_sum += test_for_loop(random_iter);
    
    printf("Final total: %d\n", total_sum);
    
    return total_sum > 0 ? 0 : 1;
}
