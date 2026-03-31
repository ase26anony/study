/* doloop_coverage.c
 * Designed to trigger GCC's doloop optimization pattern matching
 * for (reg - 1) != 0 comparison pattern in loop-doloop.cc lines 136-150
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Volatile variables to prevent constant propagation */
static volatile int volatile_bound = 0;
static volatile int volatile_sink = 0;

/* Different loop variants to increase coverage probability */

/* Variant 1: for loop with i-- and explicit != 0 comparison */
NOOPT void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sum += i * 2;
        /* Volatile write to prevent dead code elimination */
        volatile_sink = sum;
    }
    
    /* Use result to prevent optimization */
    volatile_sink = sum;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    int acc = 0;
    
    while (--cnt != 0) {
        /* Different side effect pattern */
        acc ^= cnt;
        volatile_sink = acc;
    }
    
    volatile_sink = acc;
}

/* Variant 3: while loop with post-decrement */
NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int prod = 1;
    
    while (cnt-- != 0) {
        /* Multiplication side effect */
        prod *= (cnt + 1) % 7 + 1;
        volatile_sink = prod;
    }
    
    volatile_sink = prod;
}

/* Variant 4: do-while with pre-decrement check */
NOOPT void loop_decrement_dowhile(int n) {
    int i = n;
    int result = 0;
    
    if (i > 0) {
        do {
            result += i * i;
            volatile_sink = result;
        } while (--i != 0);
    }
    
    volatile_sink = result;
}

/* Variant 5: for loop with compound decrement */
NOOPT void loop_decrement_for_compound(int n) {
    int counter = n;
    int checksum = 0;
    
    for (; counter != 0; counter -= 1) {
        checksum += (counter & 0xFF);
        volatile_sink = checksum;
    }
    
    volatile_sink = checksum;
}

/* Variant 6: unsigned counter (might generate different RTL) */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int ucnt = n;
    unsigned int mask = 0;
    
    while (ucnt-- != 0) {
        mask |= (1 << (ucnt % 16));
        volatile_sink = mask;
    }
    
    volatile_sink = mask;
}

/* Variant 7: nested loops to create more complex patterns */
NOOPT void loop_decrement_nested(int n) {
    int i, j;
    int total = 0;
    
    for (i = n; i != 0; i--) {
        for (j = 3; j != 0; j--) {
            total += i * j;
            volatile_sink = total;
        }
    }
    
    volatile_sink = total;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Use command line or volatile to get non-constant bound */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = 1000;
    } else {
        /* Use volatile read to prevent compile-time constant */
        loop_bound = volatile_bound;
        if (loop_bound <= 0) loop_bound = 1000;
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Seed RNG for additional variability */
    srand(time(NULL));
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_for_compound(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_nested(loop_bound / 10); /* Smaller bound for nested */
    
    /* Create checksum from volatile sink to prevent optimization */
    int final_result = volatile_sink;
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
