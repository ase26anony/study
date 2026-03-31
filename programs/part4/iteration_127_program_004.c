/* test-doloop.c
 * 
 * This program is designed to trigger the specific pattern-matching logic
 * in GCC's doloop_optimize pass (loop-doloop.cc lines 136-150).
 * The pattern is: (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Use volatile to prevent constant propagation */
static volatile int global_counter = 1000;

/* Side effect accumulator */
static volatile int side_effect = 0;

/* Array to prevent dead code elimination */
static int results[4] = {0};

/* Variant 1: for loop with i-- and explicit != 0 check */
NOINLINE void loop_decrement_for(int n) {
    int i;
    for (i = n; i != 0; i--) {
        side_effect += i;
        results[0] += i;
    }
}

/* Variant 2: while loop with post-decrement != 0 check */
NOINLINE void loop_decrement_while_post(int n) {
    int cnt = n;
    while (cnt-- != 0) {
        side_effect += cnt + 1;
        results[1] += cnt + 1;
    }
}

/* Variant 3: while loop with pre-decrement != 0 check */
NOINLINE void loop_decrement_while_pre(int n) {
    int cnt = n;
    while (--cnt != 0) {
        side_effect += cnt;
        results[2] += cnt;
    }
    /* Handle last iteration */
    side_effect += 1;
    results[2] += 1;
}

/* Variant 4: do-while with explicit decrement and check */
NOINLINE void loop_decrement_do_while(int n) {
    int cnt = n;
    if (cnt > 0) {
        do {
            side_effect += cnt;
            results[3] += cnt;
        } while (--cnt != 0);
    }
}

/* Variant 5: for loop with unsigned counter (common pattern) */
NOINLINE void loop_decrement_unsigned(unsigned int n) {
    unsigned int i;
    for (i = n; i != 0; i--) {
        side_effect += (int)i;
        results[0] += (int)i;  /* Reuse results[0] */
    }
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_bound = global_counter;
    }
    
    /* Ensure reasonable bounds */
    if (loop_bound <= 0) {
        loop_bound = 100;
    }
    
    /* Reset side effects */
    side_effect = 0;
    memset(results, 0, sizeof(results));
    
    /* Execute all variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_post(loop_bound);
    loop_decrement_while_pre(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += results[i];
    }
    checksum += side_effect;
    
    printf("Checksum: %d\n", checksum);
    printf("Loop bound used: %d\n", loop_bound);
    
    return 0;
}
