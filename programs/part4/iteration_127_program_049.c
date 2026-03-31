/* doloop_coverage.c
 * Designed to trigger GCC's do-while loop optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int volatile_bound = 0;
static volatile int volatile_sink = 0;

/* Function 1: Classic for loop with i-- and != 0 condition */
NOOPT void loop_decrement_for(int n) {
    int i;
    int local_sum = 0;
    
    /* The exact pattern: (reg - 1) != 0 */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        local_sum += i * 3;
        /* Additional side effect to volatile */
        volatile_sink = i;
    }
    
    /* Use result to prevent dead code elimination */
    volatile_sink = local_sum;
}

/* Function 2: While loop with pre-decrement and explicit comparison */
NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    int local_sum = 0;
    
    while (--cnt != 0) {  /* Pattern: (reg - 1) != 0 */
        local_sum += cnt * 7;
        volatile_sink = cnt;
    }
    
    volatile_sink = local_sum;
}

/* Function 3: While loop with post-decrement */
NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int local_sum = 0;
    
    while (cnt-- != 0) {  /* Pattern: (reg - 1) != 0 after decrement */
        local_sum += cnt * 11;
        volatile_sink = cnt;
    }
    
    volatile_sink = local_sum;
}

/* Function 4: Do-while with explicit decrement and comparison */
NOOPT void loop_decrement_dowhile(int n) {
    int cnt = n;
    int local_sum = 0;
    
    if (cnt > 0) {
        do {
            local_sum += cnt * 13;
            volatile_sink = cnt;
        } while (--cnt != 0);  /* Pattern: (reg - 1) != 0 */
    }
    
    volatile_sink = local_sum;
}

/* Function 5: For loop with explicit decrement in condition */
NOOPT void loop_decrement_for_cond(int n) {
    int i = n;
    int local_sum = 0;
    
    for (; i != 0; ) {
        local_sum += i * 17;
        volatile_sink = i;
        i--;  /* Decrement happens here, but condition checks i != 0 */
    }
    
    volatile_sink = local_sum;
}

/* Function 6: Nested loops to create more complex control flow */
NOOPT void loop_decrement_nested(int n) {
    int i, j;
    int local_sum = 0;
    
    for (i = n; i != 0; i--) {
        for (j = 5; j != 0; j--) {  /* Inner loop also uses pattern */
            local_sum += i * j;
        }
        volatile_sink = i;
    }
    
    volatile_sink = local_sum;
}

/* Function 7: Loop with if condition inside */
NOOPT void loop_decrement_with_if(int n) {
    int cnt = n;
    int local_sum = 0;
    
    while (cnt != 0) {
        if (cnt % 2 == 0) {
            local_sum += cnt * 19;
        } else {
            local_sum -= cnt * 23;
        }
        volatile_sink = cnt;
        cnt--;  /* Decrement after use */
    }
    
    volatile_sink = local_sum;
}

/* Function 8: Unsigned counter to avoid signed overflow issues */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int i;
    int local_sum = 0;
    
    for (i = n; i != 0; i--) {
        local_sum += (int)i * 29;
        volatile_sink = (int)i;
    }
    
    volatile_sink = local_sum;
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
    if (loop_bound <= 0) loop_bound = 100;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants to increase coverage probability */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_for_cond(loop_bound);
    loop_decrement_nested(loop_bound / 10);  /* Smaller for nested */
    loop_decrement_with_if(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    
    /* Print volatile sink to prevent elimination of all computations */
    printf("Final volatile sink value: %d\n", volatile_sink);
    
    return 0;
}
