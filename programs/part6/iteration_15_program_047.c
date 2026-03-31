/* test_loops.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrementing pattern: (compare (plus reg -1) 0)
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate the loops
 * from early optimizations and ensure they reach the hardware loop
 * detection phase.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing pattern with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent optimization */
    }
    
    /* Loop 2: Decrementing pattern with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different bound */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Different constant to distinguish from other loops */
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    if (sum > 0) {
        /* This will always be true but compiler doesn't know that */
        printf("Loop sum: %d\n", sum);
    }
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 50;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    return 0;
}
