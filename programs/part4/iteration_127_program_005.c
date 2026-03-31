/* loop-doloop-coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching for (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int volatile_bound = 0;
static volatile int volatile_sink = 0;

/* Different loop variants to increase coverage probability */

NOINLINE void loop_decrement_for(int n) {
    volatile int sum = 0;
    /* for loop with i != 0 condition */
    for (int i = n; i != 0; i--) {
        sum += i * 3;  /* Side effect depending on counter */
        volatile_sink = sum;  /* Prevent dead code elimination */
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_predec(int n) {
    volatile int sum = 0;
    int cnt = n;
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 7;
        volatile_sink = sum;
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_postdec(int n) {
    volatile int sum = 0;
    int cnt = n;
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 11;
        volatile_sink = sum;
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_do_while(int n) {
    volatile int sum = 0;
    int cnt = n;
    /* do-while with explicit check */
    if (cnt > 0) {
        do {
            sum += cnt * 13;
            volatile_sink = sum;
        } while (--cnt != 0);
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_for_unsigned(unsigned int n) {
    volatile unsigned int sum = 0;
    /* unsigned version */
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 17;
        volatile_sink = sum;
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_with_array(int n) {
    /* Use array to create memory side effects */
    volatile int arr[1024];
    int sum = 0;
    
    for (int i = n; i != 0; i--) {
        arr[i % 1024] = i * 19;
        sum += arr[i % 1024];
    }
    volatile_sink = sum;
}

NOINLINE void loop_decrement_complex_expr(int n) {
    volatile int sum = 0;
    int cnt = n;
    
    /* More complex expression in loop condition */
    while ((cnt - 1) != -1) {  /* Equivalent to cnt != 0 */
        sum += cnt * 23;
        volatile_sink = sum;
        cnt--;
    }
    volatile_sink = sum;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_bound = volatile_bound;
        if (loop_bound == 0) {
            loop_bound = 1000;  /* Default if volatile is 0 */
        }
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing doloop pattern with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_for_unsigned((unsigned int)loop_bound);
    loop_decrement_with_array(loop_bound);
    loop_decrement_complex_expr(loop_bound);
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", volatile_sink);
    
    return 0;
}
