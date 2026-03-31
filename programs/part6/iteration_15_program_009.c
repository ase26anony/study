/* test_hwloop.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrement-and-branch pattern: (compare (plus reg -1) 0).
 * The loops are placed in a noinline function with O2 optimization to
 * ensure they reach the hardware loop detection pass.
 */

#include <stdio.h>

/* Static function with noinline attribute to isolate loops from early
 * optimizations and ensure loop optimization passes run on it.
 * The O2 optimization level is explicitly requested.
 */
static void __attribute__((noinline, optimize("O2")))
test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;  /* Prevent dead code elimination */

    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * This should generate: (compare (plus i -1) 0) */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }

    /* Loop 2: Decrementing loop with != 0 condition.
     * Also matches the canonical pattern. */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }

    /* Loop 3: Another decrementing loop with > 0 condition.
     * Increases chances of pattern matching. */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }

    /* Use sum to prevent elimination */
    (void)sum;
}

int main(void)
{
    /* Volatile bounds prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;

    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);

    /* Print a dummy value to prevent dead code elimination of main */
    printf("Loop test completed.\n");
    return 0;
}
