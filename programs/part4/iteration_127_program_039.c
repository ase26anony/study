#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural optimization */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Different loop variants targeting the (reg - 1) != 0 pattern */

NOOPT void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    /* for loop with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        sum += i * 2;
    }
    sink = sum; /* Side effect to prevent elimination */
}

NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    int sum = 0;
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 3;
    }
    sink = sum;
}

NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int sum = 0;
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 5;
    }
    sink = sum;
}

NOOPT void loop_decrement_do_while(int n) {
    int cnt = n;
    int sum = 0;
    /* do-while with explicit check */
    if (cnt > 0) {
        do {
            sum += cnt * 7;
        } while (--cnt != 0);
    }
    sink = sum;
}

NOOPT void loop_decrement_for_complex(int n) {
    unsigned int i;
    int sum = 0;
    /* Using unsigned to avoid signed overflow issues */
    for (i = (unsigned int)n; i != 0; i--) {
        /* Complex enough to prevent other optimizations */
        sum += (i & 1) ? i : -i;
    }
    sink = sum;
}

NOOPT void loop_decrement_with_if(int n) {
    int i = n;
    int sum = 0;
    /* Loop with if inside, still decrementing */
    while (i != 0) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
        i--;
    }
    sink = sum;
}

/* Helper to make loop bound non-constant at compile time */
static int get_iterations(void) {
    volatile int iterations = 1000; /* Not a compile-time constant */
    return iterations;
}

int main(int argc, char *argv[]) {
    int iterations;
    
    /* Make loop count non-constant to prevent unrolling */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    } else {
        iterations = get_iterations();
    }
    
    printf("Running loops with %d iterations\n", iterations);
    
    /* Execute all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_do_while(iterations);
    loop_decrement_for_complex(iterations);
    loop_decrement_with_if(iterations);
    
    /* Use sink to prevent elimination */
    printf("Final sink value: %d\n", sink);
    
    return 0;
}
