/* test_loops.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrement-and-branch pattern: (compare (plus reg -1) 0).
 * The loops use volatile bounds to prevent constant propagation and are
 * placed in a noinline function with O2 optimization to ensure they reach
 * the hardware loop detection pass.
 */

#include <stdio.h>

/* Static function marked noinline to isolate loops from main and ensure
 * optimization passes run on it. The O2 optimization level is specified
 * to enable loop optimizations while preventing inlining. */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int b1, volatile int b2, volatile int b3)
{
    volatile int sum = 0;  /* Volatile accumulator to create side effects */

    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected RTL pattern: (compare (plus i -1) 0) */
    for (volatile int i = b1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }

    /* Loop 2: Decrementing loop with != 0 condition.
     * Should also generate the same canonical pattern. */
    for (int j = b2; j != 0; j--) {
        sum += 2;
    }

    /* Loop 3: Another decrementing loop with > 0 condition, using a different bound.
     * Increases chances of pattern matching. */
    for (int k = b3; k > 0; k--) {
        sum += 3;
    }

    /* Use sum in a trivial way to prevent dead code elimination */
    if (sum > 0) {
        /* This branch is always taken but compiler doesn't know it */
        printf("Dummy print to prevent elimination: %d\n", sum);
    }
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;

    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);

    return 0;
}
