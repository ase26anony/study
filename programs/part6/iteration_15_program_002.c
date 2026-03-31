/* test_hwloop.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrement-and-branch pattern: (compare (plus reg -1) 0).
 * The loops are placed in a noinline function with O2 optimization
 * to ensure they reach the hardware loop detection pass.
 */

#include <stdio.h>

/* Static function marked noinline to prevent inlining and ensure
 * optimization passes run on the isolated function body.
 * The O2 optimization level enables loop optimizations including
 * hardware loop detection.
 */
static void __attribute__((noinline, optimize("O2")))
test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;  /* Prevent dead code elimination */

    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected RTL pattern: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }

    /* Loop 2: Decrementing loop with != 0 condition.
     * Also generates the same canonical pattern.
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }

    /* Loop 3: Another decrementing loop with > 0 condition,
     * using a different variable name.
     */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }

    /* Use sum to prevent complete optimization */
    if (sum > 1000) {
        printf("unlikely\n");  /* Dummy side effect */
    }
}

int main(void)
{
    /* Volatile bounds prevent constant propagation and unrolling
     * before the loop analysis passes.
     */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;

    test_loops(bound1, bound2, bound3);

    return 0;
}
