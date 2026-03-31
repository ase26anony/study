/* loop-doloop-test.c
 * Test program to trigger GCC's doloop optimization pattern matching
 * Specifically targets the RTL pattern: (set (reg) (compare (plus (reg) -1) 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa))
int test_for_loop(int limit) {
    int sum = 0;
    /* Classic decrementing for loop: i > 0, i-- */
    for (int i = limit; i > 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* While loop with explicit decrement and comparison to 0 */
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
    /* Alternative while pattern with post-decrement */
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
    /* Unsigned version - often generates cleaner decrement patterns */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

/* Main function with volatile to prevent constant propagation */
int main(void) {
    volatile int iterations = 100;  /* Prevent constant folding */
    int total = 0;
    
    /* Call all test functions to exercise different loop patterns */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_while_postdec(iterations);
    total += test_do_while(iterations);
    total += test_unsigned_loop((unsigned int)iterations);
    
    printf("Total sum: %d\n", total);
    
    /* Also test with random value to avoid predictable patterns */
    srand(42);
    int random_iter = rand() % 1000 + 100;
    total += test_for_loop(random_iter);
    
    printf("Final total: %d\n", total);
    return total != 0 ? 0 : 1;  /* Ensure all loops executed */
}
