#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Different loop variants to increase coverage probability */

NOINLINE void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    
    /* for loop with i-- and != 0 comparison */
    for (i = n; i != 0; i--) {
        sum += i * 3;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_predec(int n) {
    int cnt = n;
    int sum = 0;
    
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 7;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int sum = 0;
    
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 11;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_do_while(int n) {
    int cnt = n;
    int sum = 0;
    
    /* do-while with pre-decrement check */
    if (cnt > 0) {
        do {
            sum += cnt * 13;
        } while (--cnt != 0);
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_for_complex(int n) {
    unsigned int i;
    int sum = 0;
    
    /* for loop with unsigned counter */
    for (i = n; i != 0; i--) {
        sum += (int)i * 17;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_nested(int n) {
    int i, j;
    int sum = 0;
    
    /* Nested loops with decrementing counters */
    for (i = n; i != 0; i--) {
        for (j = 5; j != 0; j--) {
            sum += i * j;
        }
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_with_if(int n) {
    int cnt = n;
    int sum = 0;
    
    /* Loop with internal conditional */
    while (cnt != 0) {
        if (cnt % 2 == 0) {
            sum += cnt * 19;
        } else {
            sum += cnt * 23;
        }
        cnt--;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_register_only(int n) {
    register int cnt asm ("r12") = n; /* Hint for register allocation */
    int sum = 0;
    
    /* Force counter into register with register keyword */
    while (cnt != 0) {
        sum += cnt * 29;
        cnt--;
    }
    
    volatile_sink = sum;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        volatile int seed = time(NULL);
        loop_bound = (seed % 1000) + 100;  /* 100-1099 iterations */
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_for_complex(loop_bound);
    loop_decrement_nested(loop_bound);
    loop_decrement_with_if(loop_bound);
    loop_decrement_register_only(loop_bound);
    
    /* Final checksum to prevent optimization */
    printf("Volatile sink value: %d\n", volatile_sink);
    
    return 0;
}
